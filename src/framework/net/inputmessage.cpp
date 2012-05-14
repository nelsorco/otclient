/*
 * Copyright (c) 2010-2012 OTClient <https://github.com/edubart/otclient>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "inputmessage.h"
#include "rsa.h"

InputMessage::InputMessage()
{
    reset();
}

void InputMessage::reset()
{
    m_readPos = MAX_HEADER_SIZE;
    m_headerPos = MAX_HEADER_SIZE;
}

void InputMessage::setBuffer(const std::string& buffer)
{
    memcpy(m_buffer + MAX_HEADER_SIZE, buffer.c_str(), buffer.size());
    m_readPos = MAX_HEADER_SIZE;
    m_headerPos = MAX_HEADER_SIZE;
    m_messageSize = buffer.size();
}

uint8 InputMessage::getU8(bool peek)
{
    checkRead(1);
    uint8 v = m_buffer[m_readPos];

    if(!peek)
        m_readPos += 1;

    return v;
}

uint16 InputMessage::getU16(bool peek)
{
    checkRead(2);
    uint16 v = Fw::readLE16(m_buffer + m_readPos);

    if(!peek)
        m_readPos += 2;

    return v;
}

uint32 InputMessage::getU32(bool peek)
{
    checkRead(4);
    uint32 v = Fw::readLE32(m_buffer + m_readPos);

    if(!peek)
        m_readPos += 4;

    return v;
}

uint64 InputMessage::getU64(bool peek)
{
    checkRead(8);
    uint64 v = Fw::readLE64(m_buffer + m_readPos);

    if(!peek)
        m_readPos += 8;

    return v;
}

std::string InputMessage::getString()
{
    uint16 stringLength = getU16();
    checkRead(stringLength);
    char* v = (char*)(m_buffer + m_readPos);
    m_readPos += stringLength;
    return std::string(v, stringLength);
}

void InputMessage::decryptRSA(int size, const std::string& p, const std::string& q, const std::string& d)
{
    RSA::decrypt((char*)m_buffer + m_readPos, size, p.c_str(), q.c_str(), d.c_str());
}

void InputMessage::fillBuffer(uint8 *buffer, uint16 size)
{
    memcpy(m_buffer + m_readPos, buffer, size);
    m_messageSize += size;
}

void InputMessage::setHeaderSize(uint16 size)
{
    m_headerPos = MAX_HEADER_SIZE - size;
    m_readPos = m_headerPos;
}

bool InputMessage::readChecksum()
{
    uint32_t receivedCheck = getU32();
    uint32 checksum = Fw::getAdlerChecksum(m_buffer + m_readPos, getUnreadSize());
    return receivedCheck == checksum;
}

bool InputMessage::canRead(int bytes)
{
    if((m_readPos - m_headerPos + bytes > m_messageSize) || (m_readPos + bytes > BUFFER_MAXSIZE))
        return false;
    return true;
}

void InputMessage::checkRead(int bytes)
{
    if(!canRead(bytes))
        throw NetworkException("InputMessage eof reached");
}

