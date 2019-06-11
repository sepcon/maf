IPCReceiver:
    1. open connection: Address
    2. waiting on the connection to listen for the bytes 
    3. readBytes:
        bufferOserver->onBytesCome(ByteArray)

IPCService:
    1. onBytesCome:
        IPCMessage msg = MessageValidator::createMessage(ByteArray);
        if msg is valid:
            messageReceiver->onMessage(IPCMessage);
        

file.read