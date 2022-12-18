#ifndef __NXT_H__
#define __NXT_H__

#include <cstdint>

#include <array>
#include <stdexcept>
#include <string>
#include <vector>

#include <fantom/iNXT.h>

#include "Utils.h"


namespace nxt
{

    typedef std::string ResourceName;

    struct DeviceInfo
    {
        ResourceName resourceName;
        std::string name;
        std::array<uint8_t, 6> bluetoothAddress;
        std::array<uint8_t, 4> bluetoothSignalStrengths;
        uint32_t availableFlash;
    };

    std::string bluetoothAddressToString(const std::array<uint8_t, 6>& bluetoothAddress);


    struct FirmwareVersion
    {
        ViUInt8 protocolVersionMajor;
        ViUInt8 protocolVersionMinor;
        ViUInt8 firmwareVersionMajor;
        ViUInt8 firmwareVersionMinor;
    };

    struct FileInfo
    {
        std::string filename;
        uint32_t size;
    };


    class NXT
    {
        std::unique_ptr<nFANTOM100::iNXT, void(*)(nFANTOM100::iNXT*)> m_NXT;

        friend class Factory;

        explicit NXT(nFANTOM100::iNXT* pNXT);
    public: // resource management
        NXT(const NXT& other) = delete;
        NXT(NXT&& other) noexcept;

        NXT& operator=(const NXT& other) = delete;
        NXT& operator=(NXT&& other) noexcept;

    public:
        FirmwareVersion getFirmwareVersion();
        DeviceInfo getDeviceInfo();

        std::vector<FileInfo> findFiles(const std::string& pattern = "*.*");
        bool removeFile(const std::string& filename);
        void writeFile(const std::string& filename, const std::vector<uint8_t>& contents);
        bool readFile(const std::string& filename, std::vector<uint8_t>& contents);

        template <class t_message>
        void send(const t_message& message)
        {
            nFANTOM100::tStatus status;
            m_NXT->sendDirectCommand(
                false,
                reinterpret_cast<const ViByte*>(&message), sizeof(t_message),
                nullptr, 0,
                status
            );

            if (status.isFatal())
                throw std::runtime_error(utils::S() << "Error sending direct command: " << status.getCode());
        }

        template <class t_message>
        typename t_message::Response sendAndWaitReply(const t_message& message)
        {
            nFANTOM100::tStatus status;

            typename t_message::Response response = { 0 };

            m_NXT->sendDirectCommand(
                true,
                reinterpret_cast<const ViByte*>(&message), sizeof(t_message),
                reinterpret_cast<ViPBuf>(&response), sizeof(t_message::Response),
                status
            );

            if (status.isFatal())
                throw std::runtime_error(utils::S() << "Error sending direct command: " << status.getCode());

            return response;
        }
    };


    class Factory
    {
    public:
        std::vector<DeviceInfo> getAvailableDevices(bool searchBluetooth = false) const;
        NXT create(const ResourceName& resourceName) const;
    };

}

#endif //__NXT_H__
