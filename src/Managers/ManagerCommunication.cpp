#include "ManagerCommunication.h"
#include <AltSoftSerial.h>

void ManagerCommunication::Tick(unsigned long DeltaTime)
{
    Packet packet = Packet();
    while (SoftwareSerial->available() > 0 && !packet.dataReceived() && CurrentTask.bExecuted)
    {
        uint8_t data = SoftwareSerial->read();
        packet.dataReceived(data);
    }

    if (packet.checkParity()) // Todo request resend if packet is invalid
    {
        CurrentTask = Task(packet.getData());
    }
    else
    {
        // Request packet again
    }
}

Task ManagerCommunication::GetCommand()
{
    Task ReturnTask = CurrentTask;

    CurrentTask.bExecuted = true;

    return ReturnTask;
}