#include "NdefRecord.h"

NdefRecord::NdefRecord(byte tnf, const byte *type, size_t typeLength, const byte *payload, size_t payloadLength, const byte *id, size_t idLength):
_tnf(tnf),
_typeLength(typeLength),
_payloadLength(payloadLength),
_idLength(idLength)
{
    //Serial.println("NdefRecord Constructor 1");
    if (_typeLength)
    {
        _type = (byte*)malloc(_typeLength);
        memcpy(_type, type, _typeLength);
    }

    if (_payloadLength)
    {
        _payload = (byte*)malloc(_payloadLength);
        memcpy(_payload, payload, _payloadLength);
    }

    if (_idLength)
    {
        _id = (byte*)malloc(_idLength);
        memcpy(_id, id, _idLength);
    }
}

NdefRecord::NdefRecord(const NdefRecord& rhs):
_tnf(rhs._tnf),
_typeLength(rhs._typeLength),
_payloadLength(rhs._payloadLength),
_idLength(rhs._idLength)
{
    //Serial.println("NdefRecord Constructor 2 (copy)");
    _type = _payload = _id = (byte *)NULL;

    if (_typeLength)
    {
        _type = (byte*)malloc(_typeLength);
        memcpy(_type, rhs._type, _typeLength);
    }

    if (_payloadLength)
    {
        _payload = (byte*)malloc(_payloadLength);
        memcpy(_payload, rhs._payload, _payloadLength);
    }

    if (_idLength)
    {
        _id = (byte*)malloc(_idLength);
        memcpy(_id, rhs._id, _idLength);
    }
}

NdefRecord::NdefRecord(NdefRecord&& rhs) :
_tnf(rhs._tnf),
_typeLength(rhs._typeLength),
_payloadLength(rhs._payloadLength),
_idLength(rhs._idLength),
_type(rhs._type),
_payload(rhs._payload),
_id(rhs._id)
{
    //Serial.println("NdefRecord Constructor 3 (move)");
    rhs._typeLength = rhs._payloadLength = rhs._idLength = 0;
    rhs._type = rhs._payload = rhs._id = NULL;
}

NdefRecord::~NdefRecord()
{
    //Serial.println("NdefRecord Destructor");
    if (_typeLength && _type)
    {
        free(_type);
    }

    if (_payloadLength && _payload)
    {
        free(_payload);
    }

    if (_idLength && _id)
    {
        free(_id);
    }
}

NdefRecord& NdefRecord::operator=(const NdefRecord& rhs)
{
    //Serial.println("NdefRecord ASSIGN");

    if (this != &rhs)
    {
        // free existing
        if (_typeLength && _type)
        {
            free(_type);
        }

        if (_payloadLength && _payload)
        {
            free(_payload);
        }

        if (_idLength && _id)
        {
            free(_id);
        }

        _tnf = rhs._tnf;
        _typeLength = rhs._typeLength;
        _payloadLength = rhs._payloadLength;
        _idLength = rhs._idLength;

        if (_typeLength)
        {
            _type = (byte*)malloc(_typeLength);
            memcpy(_type, rhs._type, _typeLength);
        }

        if (_payloadLength)
        {
            _payload = (byte*)malloc(_payloadLength);
            memcpy(_payload, rhs._payload, _payloadLength);
        }

        if (_idLength)
        {
            _id = (byte*)malloc(_idLength);
            memcpy(_id, rhs._id, _idLength);
        }
    }
    return *this;
}

// size of records in bytes
int NdefRecord::getEncodedSize()
{
    int size = 2; // tnf + typeLength
    if (_payloadLength > 0xFF)
    {
        size += 4;
    }
    else
    {
        size += 1;
    }

    if (_idLength)
    {
        size += 1;
    }

    size += (_typeLength + _payloadLength + _idLength);

    return size;
}

void NdefRecord::encode(byte *data, bool firstRecord, bool lastRecord)
{
    // assert data > getEncodedSize()

    uint8_t* data_ptr = &data[0];

    *data_ptr = getTnfByte(firstRecord, lastRecord);
    data_ptr += 1;

    *data_ptr = _typeLength;
    data_ptr += 1;

    if (_payloadLength <= 0xFF) {  // short record
        *data_ptr = _payloadLength;
        data_ptr += 1;
    } else { // long format
        // 4 bytes but we store length as an int
        data_ptr[0] = 0x0; // (_payloadLength >> 24) & 0xFF;
        data_ptr[1] = 0x0; // (_payloadLength >> 16) & 0xFF;
        data_ptr[2] = (_payloadLength >> 8) & 0xFF;
        data_ptr[3] = _payloadLength & 0xFF;
        data_ptr += 4;
    }

    if (_idLength)
    {
        *data_ptr = _idLength;
        data_ptr += 1;
    }

    //Serial.println(2);
    memcpy(data_ptr, _type, _typeLength);
    data_ptr += _typeLength;

    if (_idLength)
    {
        memcpy(data_ptr, _id, _idLength);
        data_ptr += _idLength;
    }
    
    memcpy(data_ptr, _payload, _payloadLength);
    data_ptr += _payloadLength;
}

byte NdefRecord::getTnfByte(bool firstRecord, bool lastRecord)
{
    int value = _tnf;

    if (firstRecord) { // mb
        value = value | 0x80;
    }

    if (lastRecord) { //
        value = value | 0x40;
    }

    // chunked flag is always false for now
    // if (cf) {
    //     value = value | 0x20;
    // }

    if (_payloadLength <= 0xFF) {
        value = value | 0x10;
    }

    if (_idLength) {
        value = value | 0x8;
    }

    return value;
}

byte NdefRecord::getTnf()
{
    return _tnf;
}

void NdefRecord::setTnf(byte tnf)
{
    _tnf = tnf;
}

unsigned int NdefRecord::getTypeLength()
{
    return _typeLength;
}

int NdefRecord::getPayloadLength()
{
    return _payloadLength;
}

unsigned int NdefRecord::getIdLength()
{
    return _idLength;
}

String NdefRecord::getType()
{
    char type[_typeLength + 1];
    memcpy(type, _type, _typeLength);
    type[_typeLength] = '\0'; // null terminate
    return String(type);
}

// this assumes the caller created type correctly
void NdefRecord::getType(uint8_t* type)
{
    memcpy(type, _type, _typeLength);
}

void NdefRecord::setType(const byte * type, const unsigned int numBytes)
{
    if(_typeLength && _type)
    {
        free(_type);
    }

    _type = (uint8_t*)malloc(numBytes);
    if (_type) {
        memcpy(_type, type, numBytes);
        _typeLength = numBytes;
    }
}

// assumes the caller sized payload properly
void NdefRecord::getPayload(byte *payload)
{
    memcpy(payload, _payload, _payloadLength);
}

void NdefRecord::setPayload(const byte * payload, const int numBytes)
{
    if (_payloadLength && _payload)
    {
        free(_payload);
    }

    _payload = (byte*)malloc(numBytes);
    if (_payload) {
        memcpy(_payload, payload, numBytes);
        _payloadLength = numBytes;
    }
}

String NdefRecord::getId()
{
    char id[_idLength + 1];
    memcpy(id, _id, _idLength);
    id[_idLength] = '\0'; // null terminate
    return String(id);
}

void NdefRecord::getId(byte *id)
{
    memcpy(id, _id, _idLength);
}

void NdefRecord::setId(const byte * id, const unsigned int numBytes)
{
    if (_idLength && _id)
    {
        free(_id);
    }

    _id = (byte*)malloc(numBytes);
    if (_id) {
        memcpy(_id, id, numBytes);
        _idLength = numBytes;
    }
}
#ifdef NDEF_USE_SERIAL

void NdefRecord::print()
{
    Serial.println(F("  NDEF Record"));
    Serial.print(F("    TNF 0x"));Serial.print(_tnf, HEX);Serial.print(" ");
    switch (_tnf) {
    case TNF_EMPTY:
        Serial.println(F("Empty"));
        break;
    case TNF_WELL_KNOWN:
        Serial.println(F("Well Known"));
        break;
    case TNF_MIME_MEDIA:
        Serial.println(F("Mime Media"));
        break;
    case TNF_ABSOLUTE_URI:
        Serial.println(F("Absolute URI"));
        break;
    case TNF_EXTERNAL_TYPE:
        Serial.println(F("External"));
        break;
    case TNF_UNKNOWN:
        Serial.println(F("Unknown"));
        break;
    case TNF_UNCHANGED:
        Serial.println(F("Unchanged"));
        break;
    case TNF_RESERVED:
        Serial.println(F("Reserved"));
        break;
    default:
        Serial.println();
    }
    Serial.print(F("    Type Length 0x"));Serial.print(_typeLength, HEX);Serial.print(" ");Serial.println(_typeLength);
    Serial.print(F("    Payload Length 0x"));Serial.print(_payloadLength, HEX);;Serial.print(" ");Serial.println(_payloadLength);
    if (_idLength)
    {
        Serial.print(F("    Id Length 0x"));Serial.println(_idLength, HEX);
    }
    Serial.print(F("    Type "));PrintHexChar(_type, _typeLength);
    // TODO chunk large payloads so this is readable
    Serial.print(F("    Payload "));PrintHexChar(_payload, _payloadLength);
    if (_idLength)
    {
        Serial.print(F("    Id "));PrintHexChar(_id, _idLength);
    }
    Serial.print(F("    Record is "));Serial.print(getEncodedSize());Serial.println(" bytes");

}
#endif
