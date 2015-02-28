#include "atemdevice.h"

#include <QSettings>

#include "../const.h"

quint16 make16b(char hbit, char lbit){
    return ((unsigned char)hbit << 8) | (unsigned char)lbit;
}

AtemDevice::AtemDevice(const Device &dev) :
    TargetableDevice(dev), _udpSocket(this) {
    _nbrAux = 6;
    _helloing = false;
    _hasInitialized = false;
    _sessionID = 0;
    _isMe2 = false;

    QTimer::singleShot(6000, this, SLOT(onTimer()));
}


AtemDevice::~AtemDevice(){
    _udpSocket.close();
    _udpSocket.abort();
}

void AtemDevice::onTimer() {
    //changeInputName(1, "COUCOU très très très très long patce que je veux test");
    setTransitionValue(5000);
}

QAbstractSocket *AtemDevice::initSocket(){
    return &_udpSocket;
}

quint8 helloStr[] = { 0x10, 0x14, 0x53, 0xAB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3A, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
quint8 helloAnswerStr[] = { 0x80, 0x0c, 0x53, 0xab, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00 };

void AtemDevice::parseInput() {
    _buffer.append(_udpSocket.readAll());
    qDebug() << "data available " << _buffer.size();
    if(_helloing) {
        qDebug() << "helloing, bytes availables: " << _buffer.size();
        if(_buffer.size() < 20){
            return;
        }
        quint8 command = _buffer[0] & 0xF8;
        qDebug() << "reading HELLO stream, size: " << _buffer.size() << "command:" << command;
        _buffer.remove(0, 20);

        qDebug() << "sending hello answer ";
        _udpSocket.write((char*) helloAnswerStr, sizeof(helloAnswerStr));
        _udpSocket.flush();
        _helloing = false;
        _buffer.clear();
    }

    while(_buffer.size() >= 12){
        //_udpSocket.read((char*)packetBuffer, 12);
       // bytesAvailable -= 12;

        _sessionID = make16b(_buffer[2], _buffer[3]);

        quint16 packetSize = make16b(_buffer[0] & 0x07, _buffer[1]);
        quint16 lastRemotePacketID = make16b(_buffer[10], _buffer[11]);
        quint8 command = _buffer[0] & 0xF8;
        qDebug() << "READING stream, buffer size: " << _buffer.size() << "packet size:" << packetSize << "command:" << command << ", cmd: " << command << QString::number(command, 2);
        qDebug() << _buffer.left(12).toHex();
        bool command_ACK = (command & 0x08) ? true : false;	// If true, ATEM expects an acknowledgement answer back!
        bool command_INIT = (command & 0x10) ? true : false;	// If true, ATEM expects an acknowledgement answer back!

        _token = make16b(_buffer[14], _buffer[15]);

        // The five bits in "command" (from LSB to MSB):
        // 1 = ACK, "Please respond to this packet" (using the _lastRemotePacketID). Exception: The initial 10-20 kbytes of Switcher status
        // 2 = ?. Set during initialization? (first hand-shake packets contains that)
        // 3 = "This is a retransmission". You will see this bit set if the ATEM switcher did not get a timely response to a packet.
        // 4 = ? ("hello packet" according to "ratte", forum at atemuser.com)
        // 5 = "This is a response on your request". So set this when answering...

        _buffer.remove(0, 12);

        //FIXME: Sam
        //if (packetSize==packetLength) { // Just to make sure these are equal, they should be!
        //    _lastContact = millis();
        //}

        // If a packet is 12 bytes long it indicates that all the initial information
        // has been delivered from the ATEM and we can begin to answer back on every request
        // Currently we don't know any other way to decide if an answer should be sent back...

        qDebug() << "inited" << _hasInitialized << ", cmdInit: " << command_INIT << ", cmdHack: " << command_ACK << ", cmd: " << command << QString::number(command, 2);

        if(!_hasInitialized && packetSize == 12) {
            qDebug() << "INIT => true";
            _hasInitialized = true;
        }

        if((packetSize > 12) && !command_INIT)	{	// !command_INIT is because there seems to be no commands in these packets and that will generate an error.
            qDebug() << "Before parsing: inited" << !_hasInitialized;
            parsePacket(packetSize);
            qDebug() << "SEND ACK";
            sendAck(lastRemotePacketID);
        }
        else if (_hasInitialized && command_ACK) {
            qDebug() << "SEND ACK 2";
            sendAck(lastRemotePacketID);
        }
    }
    _buffer.clear();
}

/**
* Sending a regular answer packet back (tell the switcher that "we heard you, thanks.")
*/
void AtemDevice::sendAck(quint16 remotePacketID) {
    quint8 packetBuffer[12];
    memset(packetBuffer, 0, sizeof(packetBuffer));	// Using 12 bytes of answer buffer, setting to zeros.
    packetBuffer[2] = _sessionID >> 8; // Session ID
    packetBuffer[3] = _sessionID & 0xFF; // Session ID
    packetBuffer[4] = remotePacketID / 256; // Remote Packet ID, MSB
    packetBuffer[5] = remotePacketID % 256; // Remote Packet ID, LSB
    packetBuffer[9] = 0x41; // ??? API

    quint16 returnPacketLength = sizeof(packetBuffer);
    packetBuffer[0] = (returnPacketLength / 256) | 0x80;
    packetBuffer[1] = returnPacketLength % 256;

    _udpSocket.write((char*)packetBuffer, sizeof(packetBuffer));
    _udpSocket.flush();
}



/**
* If a package longer than a normal acknowledgement is received from the ATEM Switcher we must read through the contents.
* Usually such a package contains updated state information about the mixer
* Selected information is extracted in this function and transferred to internal variables in this library.
*/
void AtemDevice::parsePacket(quint16 packetLength)	{
    qDebug() << "parsing packet, length = " << packetLength;


    //quint8 idx;	// General reusable index usable for keyers, mediaplayer etc below.
    // If packet is more than an ACK packet (= if its longer than 12 bytes header), lets parse it:

    //quint16 indexPointer = 12;	// 12 bytes has already been read from the packet...
    packetLength -= 12;
    while((packetLength >= 8) && (_buffer.size() >= packetLength)) {
        // Read the length of segment (first word):
        //_udpSocket.read((char*)packetBuffer, 8);

        quint16 cmdLength = make16b(_buffer[0], _buffer[1]);
        //quint16 cmdPointer = 0;
        // Get the "command string", basically this is the 4 char variable name in the ATEM memory holding the various state values of the system:
        QString cmd;
        cmd.append((char)_buffer[4]).append((char)_buffer[5]).append((char)_buffer[6]).append((char)_buffer[7]);
        _buffer.remove(0, 8);

        qDebug() << "parsing cmd " << cmd << ", length:" << cmdLength;
        qDebug() << _buffer.left(cmdLength).toHex();


        if (cmdLength <= 8) {
            return; //  length of segment should always be larger than 8 !
        }
        if(cmd == "AMLv")	{
            //_readToPacketBuffer();	// Fill packet buffer unless it's AMLv (AudioMonitorLevels)
        }
        // Extract the specific state information we like to know about:
        if(cmd == "PrgI") { // Program Bus status
            quint8 index = _buffer[0];
            if (!ver42())	{
                //_ATEM_PrgI = _packetBuffer[1];
            }
            else {
                //_ATEM_PrgI = (quint16)((_packetBuffer[2]<<8) + _packetBuffer[3]);
                qDebug() << (index == 0 ? "Program" : "M/E 2 Program") << make16b(_buffer[2], _buffer[3]);
            }
        }
        else if(cmd == "PrvI") { // Preview Bus status
            quint8 index = _buffer[0];
            if (!ver42())	{
                //_ATEM_PrvI = _packetBuffer[1];
            }
            else {
                //_ATEM_PrvI = (quint16)(_packetBuffer[2]<<8) | _packetBuffer[3];
                qDebug() << (index == 0 ? "Preview" : "M/E 2 Preview") << make16b(_buffer[2], _buffer[3]);
            }
        }
        /*else if(strcmp(cmdStr, "TlIn") == 0) { // Tally status for inputs 1-8
            quint8 count = _packetBuffer[1]; // Number of inputs
            // 16 inputs supported so make sure to read max 16.
            if(count > 16) {
            count = 16;
            }
            // Inputs 1-16, bit 0 = Prg tally, bit 1 = Prv tally. Both can be set simultaneously.
            for(quint8 i = 0; i < count; ++i) {
                _ATEM_TlIn[i] = _packetBuffer[2+i];
            }
        }
        else if(strcmp(cmdStr, "Time") == 0) { // Time. What is this anyway?
        }
        else if(strcmp(cmdStr, "TrPr") == 0) { // Transition Preview
            _ATEM_TrPr = _packetBuffer[1] > 0 ? true : false;
        }
        else if(strcmp(cmdStr, "TrPs") == 0) { // Transition Position
            _ATEM_TrPs_frameCount = _packetBuffer[2];	// Frames count down
            _ATEM_TrPs_position = _packetBuffer[4]*256 + _packetBuffer[5];	// Position 0-1000 - maybe more in later firmwares?
        }
        else if(strcmp(cmdStr, "TrSS") == 0) { // Transition Style and Keyer on next transition
            _ATEM_TrSS_KeyersOnNextTransition = _packetBuffer[2] & B11111;	// Bit 0: Background; Bit 1-4: Key 1-4
            if (_serialOutput) Serial.print(F("Keyers on Next Transition: "));
            if (_serialOutput) Serial.println(_ATEM_TrSS_KeyersOnNextTransition, BIN);
            _ATEM_TrSS_TransitionStyle = _packetBuffer[1];
            if (_serialOutput) Serial.print(F("Transition Style: "));	// 0=MIX, 1=DIP, 2=WIPE, 3=DVE, 4=STING
            if (_serialOutput) Serial.println(_ATEM_TrSS_TransitionStyle, DEC);
        }
        else if(strcmp(cmdStr, "FtbS") == 0) { // Fade To Black State
            _ATEM_FtbS_state = _packetBuffer[2]; // State of Fade To Black, 0 = off and 1 = activated
            _ATEM_FtbS_frameCount = _packetBuffer[3];	// Frames count down
            if(_serialOutput) Serial.print(F("FTB:"));
            if (_serialOutput) Serial.print(_ATEM_FtbS_state);
            if (_serialOutput) Serial.print(F("/"));
            if (_serialOutput) Serial.println(_ATEM_FtbS_frameCount);
        }
        else if(strcmp(cmdStr, "FtbP") == 0) { // Fade To Black - Positions(?) (Transition Time in frames for FTB): 0x01-0xFA
            _ATEM_FtbP_time = _packetBuffer[1];
        }
        else if(strcmp(cmdStr, "TMxP") == 0) { // Mix Transition Position(?) (Transition Time in frames for Mix transitions.): 0x01-0xFA
            _ATEM_TMxP_time = _packetBuffer[1];
        }
        else if(strcmp(cmdStr, "DskS") == 0) { // Downstream Keyer state. Also contains information about the frame count in case of "Auto"
            idx = _packetBuffer[0];
            if (idx >=0 && idx <=1)	{
                _ATEM_DskOn[idx] = _packetBuffer[1] > 0 ? true : false;
                if (_serialOutput) Serial.print(F("Dsk Keyer "));
                if (_serialOutput) Serial.print(idx+1);
                if (_serialOutput) Serial.print(F(": "));
                if (_serialOutput) Serial.println(_ATEM_DskOn[idx], BIN);
            }
        }
        else if(strcmp(cmdStr, "DskP") == 0) { // Downstream Keyer Tie
            idx = _packetBuffer[0];
            if (idx >=0 && idx <=1)	{
                _ATEM_DskTie[idx] = _packetBuffer[1] > 0 ? true : false;
                if (_serialOutput) Serial.print(F("Dsk Keyer"));
                if (_serialOutput) Serial.print(idx+1);
                if (_serialOutput) Serial.print(F(" Tie: "));
                if (_serialOutput) Serial.println(_ATEM_DskTie[idx], BIN);
            }
        }
        else if(strcmp(cmdStr, "KeOn") == 0) { // Upstream Keyer on
            idx = _packetBuffer[1];
            if (idx >=0 && idx <=3)	{
                _ATEM_KeOn[idx] = _packetBuffer[2] > 0 ? true : false;
                if (_serialOutput) Serial.print(F("Upstream Keyer "));
                if (_serialOutput) Serial.print(idx+1);
                if (_serialOutput) Serial.print(F(": "));
                if (_serialOutput) Serial.println(_ATEM_KeOn[idx], BIN);
            }
        }
        else if(strcmp(cmdStr, "ColV") == 0) { // Color Generator Change
            // Todo: Relatively easy: 8 bytes, first is the color generator, the last 6 is hsl words
        }
        else if(strcmp(cmdStr, "MPCE") == 0) { // Media Player Clip Enable
            idx = _packetBuffer[0];
            if (idx >=0 && idx <=1)	{
                _ATEM_MPType[idx] = _packetBuffer[1];
                _ATEM_MPStill[idx] = _packetBuffer[2];
                _ATEM_MPClip[idx] = _packetBuffer[3];
            }
        }*/
        else if(cmd == "AuxS") { // Aux Output Source
            quint8 auxInput = _buffer[0];
            if (!ver42())	{
                //_ATEM_AuxS[auxInput] = _packetBuffer[1];
            }
            else {
                //_ATEM_AuxS[auxInput] = (quint16)(_packetBuffer[2]<<8) | _packetBuffer[3];
                qDebug() << "Aux " << auxInput+1 << make16b(_buffer[2], _buffer[3]);
            }
        }
        else if(cmd == "_ver") { // Firmware version
            qDebug() << "Version: " << QString::number(_buffer[1]) << "." << QString::number(_buffer[3]);
        }
        else if(cmd == "InPr") { // Input
            quint16 index = make16b(_buffer[0], _buffer[1]);
            char name[256];
            for(quint8 i=0;i<cmdLength-8;i++) {
                name[i] = (char)_buffer[i+2];
            }
            name[cmdLength-8] = '\0';	// Termination
            qDebug() << "Input " << index+1 << ": " << name;
            //_ATEM_ver_m = packetBuffer[1];	// Firmware version, "left of decimal point" (what is that called anyway?)
            //_ATEM_ver_l = _packetBuffer[3];	// Firmware version, decimals ("right of decimal point")
        }
        else if(cmd == "_pin") { // Name
            char name[256];
            for(quint8 i=0;i<cmdLength-8;i++) {
                name[i] = (char)_buffer[i];
            }
            name[cmdLength-8] = '\0';	// Termination
            qDebug() << "Name:" << name;
        }
        else if(cmd == "AMTl") { // Audio Monitor Tally (on/off settings)
        // Same system as for video: "TlIn"... just implement when time.
        } // Note for future reveng: For master control, volume at least comes back in "AMMO" (CAMM is the command code.)
        else if(cmd == "AMIP") { // Audio Monitor Input P... (state) (On, Off, AFV)
            //SAM
            /*
            if (_packetBuffer[1]<13)	{
                _ATEM_AudioChannelMode[_packetBuffer[1]] = _packetBuffer[8];
                // 0+1 = Channel (high+low byte)
                // 6 = On/Off/AFV
                // 10+11 = Balance (0xD8F0 - 0x0000 - 0x2710)
                // 8+9 = Volume (0x0020 - 0xFF65)
            } /SAM */
            /* for(quint8 a=0;a<_cmdLength-8;a++) {
            Serial.print((quint8)_packetBuffer[a], HEX);
            Serial.print(" ");
            }
            Serial.println("");
            */
            /* 1M/E:
            0: MASTER
            1: (Monitor?)
            2-9: HDMI1 - SDI8
            10: MP1
            11: MP2
            12: EXT
            TVS:
            0: MASTER
            1: (Monitor?)
            2-7: INPUT1-6 (HDMI - HDMI - HDMI/SDI - HDMI/SDI - SDI - SDI)
            8: EXT
            */
            /* Serial.print("Audio Channel: ");
            Serial.println(_packetBuffer[0]); // _packetBuffer[2] seems to be input number (one higher...)
            Serial.print(" - State: ");
            Serial.println(_packetBuffer[3] == 0x01 ? "ON" : (_packetBuffer[3] == 0x02 ? "AFV" : (_packetBuffer[3] > 0 ? "???" : "OFF")));
            Serial.print(" - Volume: ");
            Serial.print((quint16)_packetBuffer[4]*256+_packetBuffer[5]);
            Serial.print("/");
            Serial.println((quint16)_packetBuffer[6]*256+_packetBuffer[7]);
            */
        }
        else if(cmd == "AMLv") { // Audio Monitor Levels
            // Get number of channels:
           /* _readToPacketBuffer(4);	// AMLv (AudioMonitorLevels)
            quint8 numberOfChannels = _packetBuffer[1];
            quint8 readingOffset=0;
            _readToPacketBuffer(32);	// AMLv (AudioMonitorLevels)
            if (_ATEM_AMLv_channel<=1)	{	// Master or Monitor vol
                readingOffset= _ATEM_AMLv_channel<<4;
                _ATEM_AMLv[0] = ((quint16)(_packetBuffer[readingOffset+1]<<8) | _packetBuffer[readingOffset+2]);	//drops the 8 least sign. bits! -> 15 bit resolution for VU purposes. fine enough.
                readingOffset+=4;
                _ATEM_AMLv[1] = ((quint16)(_packetBuffer[readingOffset+1]<<8) | _packetBuffer[readingOffset+2]);	//drops the 8 least sign. bits! -> 15 bit resolution for VU purposes. fine enough.
            }
            else {
                // Match indexes to input src numbers:
                _readToPacketBuffer(numberOfChannels & 1 ? (numberOfChannels+1)<<1 : numberOfChannels<<1);	// The block of input source numbers is always divisible by 4 bytes, so we must read a multiplum of 4 at all times
                for(quint8 j=0; j<numberOfChannels; j++)	{
                // quint16 inputNum = ((quint16)(_packetBuffer[j<<1]<<8) | _packetBuffer[(j<<1)+1]);
                // Serial.println(inputNum);
                // 0x07D1 = 2001 = MP1
                // 0x07D2 = 2002 = MP2
                // 0x03E9 = 1001 = EXT
                // 0x04b1 = 1201 = RCA
                }
                // Get level data for each input:
                for(quint8 j=0; j<numberOfChannels; j++)	{
                    _readToPacketBuffer(16);
                    if (_ATEM_AMLv_channel == j+3)	{
                        readingOffset = 0;
                        _ATEM_AMLv[0] = ((quint16)(_packetBuffer[readingOffset+1]<<8) | _packetBuffer[readingOffset+2]);	//drops the 8 least sign. bits! -> 15 bit resolution for VU purposes. fine enough.
                        readingOffset+=4;
                        _ATEM_AMLv[1] = ((quint16)(_packetBuffer[readingOffset+1]<<8) | _packetBuffer[readingOffset+2]);	//drops the 8 least sign. bits! -> 15 bit resolution for VU purposes. fine enough.
                    }
                }
            }*/
        }
        else if(cmd == "VidM") { // Video format (SD, HD, framerate etc.)
            //_ATEM_VidM = _packetBuffer[0];
        }
        else {
            // SHOULD ONLY THE UNCOMMENTED for development and with a high Baud rate on serial - 115200 for instance. Otherwise it will not connect due to serial writing speeds.
            /*
            if (_serialOutput) {
            Serial.print(("???? Unknown token: "));
            Serial.print(cmdStr);
            Serial.print(" : ");
            }
            for(quint8 a=(-2+8);a<_cmdLength-2;a++) {
            if (_serialOutput && (quint8)_packetBuffer[a]<16) Serial.print(0);
            if (_serialOutput) Serial.print((quint8)_packetBuffer[a], HEX);
            if (_serialOutput) Serial.print(" ");
            }
            if (_serialOutput) Serial.println("");
            */
        }
        // Empty, if long packet and not read yet:
        //while (_readToPacketBuffer())	{}
        //indexPointer+=_cmdLength;
        _buffer.remove(0, cmdLength - 8);
        packetLength -= cmdLength;
    }
    _buffer.remove(0, packetLength);
}

bool AtemDevice::isConfigurable() const {
    return true;
}


void AtemDevice::ping() {

}

quint16 AtemDevice::port() const {
    return ATEM_PORT;
}

int AtemDevice::pingDelay() const {
    return -1;
}

quint32 AtemDevice::pingLostTolerance() const {
    return 2;
}

int AtemDevice::reconnectDelay() const {
    return 2000;
}

QMap<quint16, QString> AtemDevice::getOutputs() const {
    QMap<quint16, QString> result;
    if(_isMe2){
        result.insert((quint16) Program, "M/E 1 Program");
        result.insert((quint16) Preview, "M/E 1 Preview");
        result.insert((quint16) ME2, "M/E 2 Program");
        result.insert((quint16) ME2Preview, "M/E 2 Preview");
    }
    else {
        result.insert((quint16) Program, "Program");
        result.insert((quint16) Preview, "Preview");
    }

    for(quint16 i = 0; i < _nbrAux; i++){
        result.insert(((quint16)AuxFirst)+i, "Aux "+QString::number(i+1));
    }
    return result;
}

QMap<quint16, QString> AtemDevice::getInputs() const {
    return QMap<quint16, QString>();
}

void AtemDevice::onCnxEstablished() {
    _helloing = true;
    _hasInitialized = false;
    _sessionID = 0;
    _localPacketIdCounter = 1;
    _udpSocket.write((char*)helloStr, sizeof(helloStr));
    _udpSocket.flush();
}

bool AtemDevice::ver42() {
    //FIXME
    return true;
}

bool AtemDevice::isConfigurableNow() const {
    //FIXME
    return true;
}

void AtemDevice::setInputName(quint16 index, QString name) {
    //FIXME
}

void AtemDevice::loadSpecific(QSettings &settings) {
    ConnectedDevice::loadSpecific(settings);
    _isMe2 = settings.value("me2",false).toBool();
    _nbrAux = settings.value("nbrAux", 3).toUInt();
    ConnectedDevice::saveSpecific(settings);

    settings.beginGroup("inputs");
    const QStringList childKeys = settings.childKeys();
    bool ok;
    foreach(const QString &childKey, childKeys) {
        quint16 inputIndex = childKey.toUInt(&ok);
        if(!ok){
            continue;
        }
        _inputLabels.insert(inputIndex, settings.value(childKey, "Input "+QString::number(inputIndex+1)).toString());
    }
    settings.endGroup();
}

void AtemDevice::saveSpecific(QSettings &settings) {
    ConnectedDevice::saveSpecific(settings);
    settings.setValue("me2", _isMe2);
    settings.setValue("nbrAux", _nbrAux);
    settings.beginGroup("inputs");
    foreach(quint16 index, _inputLabels.keys()){
        settings.setValue(QString::number(index), _inputLabels[index]);
    }
    settings.endGroup();

}

void AtemDevice::changeProgramInput(quint16 inputNumber) {
    qDebug() << "sending data !";
    bool prg = false;
    if(prg){
        QByteArray sendBuffer(4, (quint8)0);
        sendBuffer[0] = (quint8)1;
        sendBuffer[2] = (inputNumber >> 8);
        sendBuffer[3] = (inputNumber & 0xFF);

        sendPacketBufferCmdData("CPgI", sendBuffer); //CPvI
    }
    else {
        QByteArray sendBuffer(8, (quint8)0);
        //sendBuffer[0] = (quint8)0;
        sendBuffer[0] = (quint8)1;
        sendBuffer[1] = (quint8)2;
        sendBuffer[2] = (inputNumber >> 8);
        sendBuffer[3] = (inputNumber & 0xFF);

        sendPacketBufferCmdData("CAuS", sendBuffer);
        //uint8_t commandBytes[8] = {0x01, auxOutput-1, , inputNumber & 0xFF, 0,0,0,0};
        //_sendCommandPacket("CAuS", commandBytes, 8);
    }
}

void AtemDevice::setTransitionValue(quint16 value) {

    //00: M/e1, 01: Me/2
    quint8 MeSelect = 0x01;

    //Mix
    QByteArray sendBuffer(4, (quint8)0);
    sendBuffer[0] = (quint8)0x01;
    sendBuffer[1] = MeSelect;
    sendBuffer[2] = (quint8)0x00;
    sendBuffer[3] = (quint8)0x02;
    sendPacketBufferCmdData("CTTp", sendBuffer);

    QByteArray sendBuffer2(4, (quint8)0);
    sendBuffer2[0] = (quint8)0x02;
    sendBuffer2[1] = MeSelect;
    sendBuffer2[2] = (quint8)0x6a;
    sendBuffer2[3] = (quint8)0x01;
    sendPacketBufferCmdData("CTTp", sendBuffer2);

    QByteArray sendBuffer3(4, (quint8)0);
    sendBuffer3[0] = MeSelect;
    sendBuffer3[1] = (quint8)0xe4;
    sendBuffer3[2] = (value >> 8);
    sendBuffer3[3] = (value)%256;
    sendPacketBufferCmdData("CTPs", sendBuffer3);
}

void AtemDevice::changeInputName(quint16 input, const QString &name) {
    QByteArray sendBuffer(32, (quint8)0);

    QByteArray namearray = name.toLatin1();
    namearray.resize(20);

    sendBuffer[0] = (quint8)1;
    sendBuffer[1] = (input >> 8);
    sendBuffer[2] = (input & 0xFF);
    sendBuffer.replace(3, 21, namearray);
    sendBuffer[28] = (quint8)0xFF;
    sendPacketBufferCmdData("CInL", sendBuffer);
/*
    QByteArray sendBuffer2(32, (quint8)0);

    QByteArray namearray2 = name.toLatin1();
    namearray2.resize(4);

    sendBuffer2[0] = (quint8)2;
    sendBuffer2[1] = (input >> 8);
    sendBuffer2[2] = (input & 0xFF);
    sendBuffer2.replace(24, 4, namearray2);
    sendPacketBufferCmdData("CInL", sendBuffer2);*/
}

void AtemDevice::sendPacketBufferCmdData(const char cmd[4], const QByteArray &data) {
    //Answer packet preparations:
    QByteArray sendBuffer(20, (quint8)0);
    sendBuffer.append(data);

    sendBuffer[2] = _sessionID >> 8; // Session ID
    sendBuffer[3] = _sessionID & 0xFF; // Session ID
    sendBuffer[10] = _localPacketIdCounter/256; // Remote Packet ID, MSB
    sendBuffer[11] = _localPacketIdCounter%256; // Remote Packet ID, LSB

    for(quint16 i=0; i<4; i++) {
        sendBuffer[12+4+i] = cmd[i];
    }
    // Command length:
    sendBuffer[12] = (4+4+data.size())/256;
    sendBuffer[12+1] = (4+4+data.size())%256;

    quint16 token = 0; //_token ?
    sendBuffer[14] = token/256;
    sendBuffer[15] = token%256;

    // Create header:
    quint16 returnPacketLength = 20+data.size();
    sendBuffer[0] = (returnPacketLength/256) | (quint8)0x08;
    sendBuffer[1] = returnPacketLength%256;

    // Send connectAnswerString to ATEM:
    _udpSocket.write(sendBuffer);
    _udpSocket.flush();
    ++_localPacketIdCounter;
}
