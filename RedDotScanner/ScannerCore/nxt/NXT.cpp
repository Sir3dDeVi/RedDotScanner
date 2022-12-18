#include "NXT.h"

#include <cassert>
#include <cstring>

#include <iomanip>
#include <memory>
#include <sstream>


namespace nxt
{

std::string bluetoothAddressToString(const std::array<uint8_t, 6>& bluetoothAddress)
{
	std::ostringstream ss;
	ss
		<< std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(bluetoothAddress[0]) << ":"
		<< std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(bluetoothAddress[1]) << ":"
		<< std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(bluetoothAddress[2]) << ":"
		<< std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(bluetoothAddress[3]) << ":"
		<< std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(bluetoothAddress[4]) << ":"
		<< std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(bluetoothAddress[5]);

	return ss.str();
}

// NXT /////////////////////////////////////////////////////////////////////////


NXT::NXT(nFANTOM100::iNXT* pNXT)
	: m_NXT(pNXT, nFANTOM100::iNXT::destroyNXT)
{
}

NXT::NXT(NXT&& other) noexcept
	: m_NXT(nullptr, nFANTOM100::iNXT::destroyNXT)
{
	*this = std::move(other);
}

NXT& NXT::operator=(NXT&& other) noexcept
{
	if (this != &other) {
		m_NXT = std::move(other.m_NXT);
	}

	return *this;
}


FirmwareVersion NXT::getFirmwareVersion()
{
	nFANTOM100::tStatus status;

	FirmwareVersion fv = { 0 };
	m_NXT->getFirmwareVersion(fv.protocolVersionMajor, fv.protocolVersionMinor, fv.firmwareVersionMajor, fv.firmwareVersionMinor, status);
	assert(status.isSuccess());

	return fv;
}

DeviceInfo NXT::getDeviceInfo()
{
	nFANTOM100::tStatus status;
	
	ViChar szResourceName[256] = { '\0' };
	m_NXT->getResourceString(szResourceName, status);
	assert(status.isSuccess());

	ViChar szName[16] = { '\0' };
	ViByte aBluetoothAddress[6] = { 0 };
	ViUInt8 aSignalStrength[4] = { 0 };
	ViUInt32 availableFlash = 0;
	m_NXT->getDeviceInfo(szName, aBluetoothAddress, aSignalStrength, availableFlash, status);
	assert(status.isSuccess());

	DeviceInfo di;
	di.resourceName = szResourceName;
	di.name = szName;
	memcpy(&di.bluetoothAddress[0], aBluetoothAddress, 6);
	memcpy(&di.bluetoothSignalStrengths[0], aSignalStrength, 4);
	di.availableFlash = availableFlash;
	return di;
}

std::vector<FileInfo> NXT::findFiles(const std::string& pattern)
{
	std::vector<FileInfo> files;

	nFANTOM100::tStatus status;
	nFANTOM100::iFileIterator* pFileIterator = m_NXT->createFileIterator(pattern.c_str(), status);

	while (status.isSuccess()) {
		ViChar szFileName[20] = { '\0' };
		pFileIterator->getName(szFileName, status);
		assert(status.isSuccess());

		ViUInt32 size = pFileIterator->getSize(status);
		assert(status.isSuccess());

		FileInfo fileInfo;
		fileInfo.filename = szFileName;
		fileInfo.size = size;
		files.push_back(fileInfo);

		pFileIterator->advance(status);
	}
	
	return files;
}

bool NXT::removeFile(const std::string& filename)
{
	nFANTOM100::tStatus status;
	nFANTOM100::iFile* pFile = m_NXT->createFile(filename.c_str(), status);
	assert(status.isSuccess());

	pFile->remove(status);
	
	m_NXT->destroyFile(pFile);

	return status.isSuccess();
}

void NXT::writeFile(const std::string& filename, const std::vector<uint8_t>& contents)
{
	nFANTOM100::tStatus status;
	nFANTOM100::iFile* pFile = m_NXT->createFile(filename.c_str(), status);
	assert(status.isSuccess());

	pFile->openForWrite(contents.size(), status);
	if (status.isFatal()) {
		status.clear();
		pFile->remove(status);
		assert(status.isSuccess());

		pFile->openForWrite(contents.size(), status);
		assert(status.isSuccess());
	}

	pFile->write(&contents[0], contents.size(), status);
	assert(status.isSuccess());

	pFile->close(status);

	m_NXT->destroyFile(pFile);
}

bool NXT::readFile(const std::string& filename, std::vector<uint8_t>& contents)
{
	nFANTOM100::tStatus status;
	nFANTOM100::iFile* pFile = m_NXT->createFile(filename.c_str(), status);
	assert(status.isSuccess());

	bool ret = true;
	pFile->openForRead(status);
	ret = status.isSuccess();

	if (ret) {
		const ViUInt32 size = pFile->getSize(status);
		assert(status.isSuccess());

		contents.resize(size);
		pFile->read(&contents[0], size, status);
		assert(status.isSuccess());

		pFile->close(status);
	}

	m_NXT->destroyFile(pFile);

	return ret;
}


// Factory /////////////////////////////////////////////////////////////////////


std::vector<DeviceInfo> Factory::getAvailableDevices(bool searchBluetooth) const
{
	std::vector<DeviceInfo> result;

	nFANTOM100::tStatus itStatus;
	std::unique_ptr<nFANTOM100::iNXTIterator, void(*)(nFANTOM100::iNXTIterator*)> it(nFANTOM100::iNXT::createNXTIterator(searchBluetooth, 0, itStatus), nFANTOM100::iNXT::destroyNXTIterator);

	for ( ; itStatus.isSuccess(); it->advance(itStatus)) {
		nFANTOM100::tStatus status;

		ViChar szResourceName[256] = { '\0' };
		it->getName(szResourceName, status);
		assert(status.isSuccess());

		NXT nxt = create(szResourceName);
		result.push_back(nxt.getDeviceInfo());
	}

	return result;
}

NXT Factory::create(const ResourceName& resourceName) const
{
	nFANTOM100::tStatus status;
	nFANTOM100::iNXT* pNXT = nFANTOM100::iNXT::createNXT(resourceName.c_str(), status);
	if (status.isSuccess())
		return NXT(pNXT);
	else
		throw std::runtime_error(utils::S() << "Cannot create NXT (" << resourceName << ")!");
}

}