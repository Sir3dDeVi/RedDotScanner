#ifndef __NXTCOMMANDS_H__
#define __NXTCOMMANDS_H__

#include <cstdint>

#include <string>


namespace nxt
{

    enum class OutputPort : uint8_t { A = 0x00, B = 0x01, C = 0x02, All = 0xff };

    enum class MotorMode : uint8_t { Coast = 0x00, MotorOn = 0x01, Brake = 0x02, Regulated = 0x04 };
    inline MotorMode operator|(MotorMode a, MotorMode b) {
        return static_cast<MotorMode>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
    }

    enum class RegulationMode : uint8_t { Idle = 0x00, MotorSpeed = 0x01, MotorSync = 0x02 };

    enum class MotorRunState : uint8_t { Idle = 0x00, RampUp = 0x10, Running = 0x20, RampDown = 0x40 };

    enum class Mailbox : uint8_t { _1, _2, _3, _4, _5, _6, _7, _8, _9, _10 };


    // StartProgram ////////////////////////////////////////////////////////////////

    #pragma pack(push, 1)
    struct StartProgram
    {
        struct Response
        {
            uint8_t messageType; /* 0x00 */
            uint8_t status;
        };

    public:
        const uint8_t messageType; /* 0x00 */
        char filename[20];

        StartProgram(const std::string& file)
            : messageType(0x00)
        {
            memset(filename, 0, sizeof(filename));

            const size_t filenameLen = std::min<size_t>(file.size(), sizeof(filename));
            memcpy(filename, file.c_str(), filenameLen);
            filename[filenameLen] = '\0';
        }
    };
    #pragma pack(pop)


    // StopProgram /////////////////////////////////////////////////////////////////

    #pragma pack(push, 1)
    struct StopProgram
    {
        struct Response
        {
            uint8_t messageType; /* 0x01 */
            uint8_t status;
        };

    public:
        const uint8_t messageType; /* 0x01 */

        StopProgram()
            : messageType(0x01)
        {
        }
    };
    #pragma pack(pop)


    // SetOutputState //////////////////////////////////////////////////////////////

    #pragma pack(push, 1)
    struct SetOutputState
    {
        struct Response
        {
            uint8_t messageType; /* 0x04 */
            uint8_t status;
        };

    public:
        const uint8_t messageType; /* 0x04 */
        OutputPort port;
        int8_t power; /* -100..100 */
        MotorMode mode;
        RegulationMode regulation;
        int8_t turnRatio; /* -100..100 */
        MotorRunState runState;
        uint32_t tachoLimit; /* 0: run forever */

        SetOutputState(OutputPort port, int8_t power, MotorMode mode, RegulationMode regulation, int8_t turnRatio, MotorRunState runState, uint32_t tachoLimit)
            : messageType(0x04)
            , port(port)
            , power(power)
            , mode(mode)
            , regulation(regulation)
            , turnRatio(turnRatio)
            , runState(runState)
            , tachoLimit(tachoLimit)
        {
        }
    };
    #pragma pack(pop)


    // GetOutputState //////////////////////////////////////////////////////////////

    #pragma pack(push, 1)
    struct GetOutputState
    {
        struct Response
        {
            uint8_t messageType; /* 0x06 */
            uint8_t status;
            OutputPort port;
            int8_t power;
            MotorMode mode;
            RegulationMode regulation;
            int8_t turnRatio;
            MotorRunState runState;
            uint32_t tachoLimit;
            int32_t tachoCount;
            int32_t blockTachoCount;
            int32_t rotationCount;
        };

    public:
        const uint8_t messageType; /* 0x06 */
        OutputPort port;

        GetOutputState(OutputPort port)
            : messageType(0x06)
            , port(port)
        {
        }
    };
    #pragma pack(pop)


    // MessageWrite ////////////////////////////////////////////////////////////////

    #pragma pack(push, 1)
    struct MessageWrite
    {
        struct Response
        {
            uint8_t messageType; /* 0x09 */
            uint8_t status;
        };
    public:
        const uint8_t messageType; /* 0x09 */
        uint8_t inboxNumber;
        uint8_t messageSize;
        char message[59];

        MessageWrite(uint8_t inboxNumber, const std::string& msg)
            : messageType(0x09)
            , inboxNumber(inboxNumber)
            , messageSize(0)
        {
            memset(message, 0, sizeof(message));

            messageSize = static_cast<uint8_t>(std::min<size_t>(msg.size() + 1, 59)); // +1 = terminating NULL
            memcpy(message, msg.c_str(), messageSize);
            message[messageSize - 1] = '\0';
        }
    };
    #pragma pack(pop)


    // ResetMotorPosition //////////////////////////////////////////////////////////

    #pragma pack(push, 1)
    struct ResetMotorPosition
    {
        struct Response
        {
            uint8_t messageType; /* 0x0a */
            uint8_t status;
        };

    public:
        const uint8_t messageType; /* 0x0a */
        OutputPort port;
        bool relative;

        ResetMotorPosition(OutputPort port, bool relative)
            : messageType(0x0a)
            , port(port)
            , relative(relative)
        {
        }
    };
    #pragma pack(pop)


    // GetBatteryLevel /////////////////////////////////////////////////////////////

    #pragma pack(push, 1)
    struct GetBatteryLevel
    {
        struct Response
        {
            uint8_t messageType; /* 0x0b */
            uint8_t status;
            uint16_t voltageInMilliVolts;
        };

    public:
        const uint8_t messageType; /* 0x0b */

        GetBatteryLevel()
            : messageType(0x0b)
        {
        }
    };
    #pragma pack(pop)


    // KeepAlive ///////////////////////////////////////////////////////////////////

    #pragma pack(push, 1)
    struct KeepAlive
    {
        struct Response
        {
            uint8_t messageType; /* 0x06 */
            uint8_t status;
            uint32_t sleepTimeLimitInMilliSeconds;
        };

    public:
        const uint8_t messageType; /* 0x0d */

        KeepAlive()
            : messageType(0x0d)
        {
        }
    };
    #pragma pack(pop)


    // GetCurrentProgramName ///////////////////////////////////////////////////////

    struct GetCurrentProgramName
    {
        struct Response
        {
            uint8_t messageType; /* 0x11 */
            uint8_t status;
            char filename[20];
        };

    public:
        const uint8_t messageType; /* 0x11 */

        GetCurrentProgramName()
            : messageType(0x11)
        {
        }
    };


    // MessageRead /////////////////////////////////////////////////////////////////

    #pragma pack(push, 1)
    struct MessageRead
    {
        struct Response
        {
            uint8_t messageType; /* 0x13 */
            uint8_t status;
            uint8_t localInboxNumber;
            uint8_t messageSize;
            char message[59];
        };
    public:
        const uint8_t messageType; /* 0x13 */
        uint8_t remoteInboxNumber;
        uint8_t localInboxNumber;
        bool remove;

        MessageRead(uint8_t remoteInboxNumber, uint8_t localInboxNumber, bool remove)
            : messageType(0x13)
            , remoteInboxNumber(remoteInboxNumber)
            , localInboxNumber(localInboxNumber)
            , remove(remove)
        {
        }
    };
    #pragma pack(pop)

}

#endif // __NXTCOMMANDS_H__
