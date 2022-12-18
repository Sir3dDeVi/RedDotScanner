#include "NXTLaserTurret.h"

#include <cassert>

#include <functional>
#include <sstream>
#include <thread>

#include "nxt/NXTCommands.h"
#include "Utils.h"


namespace scanner
{

const char* NXTLaserTurret::s_szFirmwareName = "LaserScanner.rxe";
const std::chrono::milliseconds NXTLaserTurret::s_FirmwareWarmupTime(100);

const uint8_t NXTLaserTurret::s_nControlMailbox = static_cast<uint8_t>(nxt::Mailbox::_1);
const std::chrono::milliseconds NXTLaserTurret::s_ControlMailboxReadTimeout(100);

const char* NXTLaserTurret::s_szOutputPortNames[] = { "A", "B", "C" };
const size_t NXTLaserTurret::s_nOutputPortLaser = 0; // A
const size_t NXTLaserTurret::s_nOutputPortPan = 2; // C
const size_t NXTLaserTurret::s_nOutputPortTilt = 1; // B

const uint8_t NXTLaserTurret::s_nUpdateMailbox = static_cast<uint8_t>(nxt::Mailbox::_2);
const std::chrono::milliseconds NXTLaserTurret::s_UpdateThreadSleepTime(250);
const std::chrono::milliseconds NXTLaserTurret::s_KeepAliveTimeout(10000);


NXTLaserTurret::NXTLaserTurret(nxt::NXT&& nxt)
	: m_NXT(std::move(nxt))
	, m_bThreadRunning(false)
	, m_lastKeepAlive(std::chrono::high_resolution_clock::now())
{
	// Stop any running program
	auto currentProgramName = m_NXT.sendAndWaitReply(nxt::GetCurrentProgramName());
	if (currentProgramName.status == 0) {
		m_NXT.sendAndWaitReply(nxt::StopProgram());
	}

	// Upload firmware
	auto firmware = utils::readFile(s_szFirmwareName);

	auto files = m_NXT.findFiles(s_szFirmwareName);
	bool firmwareUpToDate = files.size() == 1 && files[0].size == firmware.size();

	if (!firmwareUpToDate) {
		m_NXT.writeFile(s_szFirmwareName, firmware);
	}

	// Start firmware
	auto startProgram = m_NXT.sendAndWaitReply(nxt::StartProgram(s_szFirmwareName));
	assert(startProgram.status == 0);

	// Let the program start, so it can receive the messages
	std::this_thread::sleep_for(s_FirmwareWarmupTime);


	m_bThreadRunning = true;
	m_Thread = std::thread(std::bind(&NXTLaserTurret::threadFunc, this));
}

NXTLaserTurret::~NXTLaserTurret()
{
	if (m_bThreadRunning) {
		m_bThreadRunning = false;
		m_Thread.join();
	}

	try {
		m_NXT.sendAndWaitReply(nxt::StopProgram());
	}
	catch (...) {
	}
}

NXTLaserTurret::MotorState NXTLaserTurret::getMotorState(const char* szOutputPort)
{
	using clock = std::chrono::high_resolution_clock;

	m_NXT.sendAndWaitReply(nxt::MessageWrite(s_nControlMailbox, szOutputPort));

	auto timeout = clock::now() + s_ControlMailboxReadTimeout;
	nxt::MessageRead::Response response;
	do {
		response = m_NXT.sendAndWaitReply(nxt::MessageRead(s_nControlMailbox + 10, s_nControlMailbox, true));
	} while (response.messageSize == 0 && timeout > clock::now());

	if (response.status != 0) {
		throw std::runtime_error("Reading MotorState timed out!");
	}
#if 0
	else {
		auto d = std::chrono::duration_cast<std::chrono::milliseconds>(s_MailboxReadTimeout - (timeout - clock::now()));
		std::cout << d << std::endl;
	}
#endif

	std::istringstream ss(response.message);

	MotorState state;
	ss >> state.speed >> state.position;

	return state;
}

void NXTLaserTurret::threadFunc()
{
	utils::setThreadName("NXTLaserTurret Thread");

	while (m_bThreadRunning) {
		nxt::GetOutputState::Response ports[3];

		{
			std::lock_guard<std::mutex> guard(m_Mutex);

			// Send KeepAlive periodically
			auto now = std::chrono::high_resolution_clock::now();
			if ((now - m_lastKeepAlive) >= s_KeepAliveTimeout) {
				m_NXT.sendAndWaitReply(nxt::KeepAlive());
				m_lastKeepAlive = now;
			}

			// Query ports
			ports[0] = m_NXT.sendAndWaitReply(nxt::GetOutputState(nxt::OutputPort::A));
			ports[1] = m_NXT.sendAndWaitReply(nxt::GetOutputState(nxt::OutputPort::B));
			ports[2] = m_NXT.sendAndWaitReply(nxt::GetOutputState(nxt::OutputPort::C));
		}

		invoke(&INXTLaserTurretCallback::onNXTUpdate,
			ports[s_nOutputPortLaser].power,
			ports[s_nOutputPortPan].tachoCount, ports[s_nOutputPortPan].power,
			ports[s_nOutputPortTilt].tachoCount, ports[s_nOutputPortTilt].power
		);

		std::this_thread::sleep_for(s_UpdateThreadSleepTime);
	}
}

void NXTLaserTurret::setLaser(uint8_t power)
{
	std::lock_guard<std::mutex> guard(m_Mutex);

	m_NXT.sendAndWaitReply(nxt::MessageWrite(s_nControlMailbox,
		utils::S() << s_szOutputPortNames[s_nOutputPortLaser] << " " << static_cast<int>(power)
	));
}

uint8_t NXTLaserTurret::getLaser()
{
	std::lock_guard<std::mutex> guard(m_Mutex);

	auto state = getMotorState(s_szOutputPortNames[s_nOutputPortLaser]);

	return state.speed;
}

void NXTLaserTurret::panRotate(uint8_t power)
{
	std::lock_guard<std::mutex> guard(m_Mutex);

	m_NXT.sendAndWaitReply(nxt::MessageWrite(s_nControlMailbox,
		utils::S() << s_szOutputPortNames[s_nOutputPortPan] << " " << static_cast<int>(power)
	));
}

void NXTLaserTurret::panRotateTo(uint8_t power, int position)
{
	std::lock_guard<std::mutex> guard(m_Mutex);

	m_NXT.sendAndWaitReply(nxt::MessageWrite(s_nControlMailbox,
		utils::S() << s_szOutputPortNames[s_nOutputPortPan] << " " << static_cast<int>(power) << " " << position
	));
}

void NXTLaserTurret::titltRotate(uint8_t power)
{
	std::lock_guard<std::mutex> guard(m_Mutex);

	m_NXT.sendAndWaitReply(nxt::MessageWrite(s_nControlMailbox,
		utils::S() << s_szOutputPortNames[s_nOutputPortTilt] << " " << static_cast<int>(power)
	));
}

void NXTLaserTurret::tiltRotateTo(uint8_t power, int position)
{
	std::lock_guard<std::mutex> guard(m_Mutex);

	m_NXT.sendAndWaitReply(nxt::MessageWrite(s_nControlMailbox,
		utils::S() << s_szOutputPortNames[s_nOutputPortTilt] << " " << static_cast<int>(power) << " " << position
	));
}

} // namespace scanner