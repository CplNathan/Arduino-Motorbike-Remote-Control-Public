#pragma once

#ifndef ManagerCommunication_H
#define ManagerCommunication_H

#include "ManagerBase.h"

class AltSoftSerial;

/*
Data will look like this, the first 8 bits are the odd parity bits, the middle 8 is the payload, the final 8 are also odd parity bits, there is probably a better way to parity check since we are working with upto 8 * 2 bits when an odd parity check only needs 1.
[Parity]   [S T R O  H H A E]   [Parity]
00000000    0 0 0 1  0 0 0 1    00000000
This would set the engine to on because E is 1, and would run because the parity checks pass.
*/

enum Command : uint8_t
{
    NONE = 0 << 0,

    CMD_Engine = 1 << 0,
    CMD_Alarm = 1 << 1,
    CMD_Hazards = 1 << 2,
    CMD_Horn = 1 << 3,

    STATE_On = 1 << 4,
    STATE_Reset = 1 << 5,
    STATE_Toggle = 1 << 6,
    STATE_Special = 1 << 7
};

struct Packet
{
private:
    bool transmissionStarted = false;
    uint8_t transmissionData = 0;
    bool transmissionEnded = false;
    bool isOddParity = false;

public:
    void dataReceived(uint8_t data)
    {
        if (!transmissionStarted)
        {
            if (data == 255 || data == 127) // 255 = 11111111 (odd) : 127 = 01111111 (even)
            {
                transmissionStarted = true;
                isOddParity = (data == 255);
            }
        }
        else if (!transmissionEnded)
        {
            transmissionData = data;
        }
        else if (!transmissionEnded && transmissionData != 0)
        {
            if ((isOddParity && data == 255) || (!isOddParity && data == 127))
            {
                transmissionEnded = true;
            }
        }
    }

    bool dataReceived()
    {
        return transmissionStarted && transmissionEnded;
    }

    bool checkParity()
    {
        if (!transmissionStarted || !transmissionEnded)
            return false;

        if (transmissionData == 0)
            return false;

        uint8_t parityPacket = transmissionData;
        for (int i = 1; i < 8; i++)
        {
            parityPacket ^= (parityPacket >> i ^ 2);
        }

        return (short)(parityPacket & 1); // Odd parity check
    }

    uint8_t getData()
    {
        return transmissionData;
    }

    Packet()
    {
    }
};

struct Task
{
    uint8_t Command;
    bool bExecuted;

    Task(uint8_t command)
    {
        this->Command = command;
    }

    Task()
    {
        this->bExecuted = false;
    }
};

class ManagerCommunication : public ManagerBase
{
public:
    virtual void Tick(unsigned long DeltaTime) override;

    Task GetCommand();

    ManagerCommunication()
    {
    }

    ManagerCommunication(AltSoftSerial *SoftwareSerial)
    {
        this->SoftwareSerial = SoftwareSerial;
        bReady = true;
    }

private:
    AltSoftSerial *SoftwareSerial;
    Task CurrentTask;
};

#endif