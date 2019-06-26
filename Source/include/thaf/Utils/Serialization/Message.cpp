#include "Message.h"


namespace Messaging
{
	namespace IPC
	{
		static constexpr size_t MESSAGE_LEAST_SIZE = sizeof(Message::Type) + sizeof(Message::IDType);
		static constexpr Message::IDType MESSAGE_ID_INVALID = std::numeric_limits<Message::IDType>::max();
		bool Message::isValid(Message::IDType id)
		{
			return id != MESSAGE_ID_INVALID;
		}

		Message::Message()
		{
			initialze();
		}

		Message::Message(const ByteArray & ba) : _bytes(ba)
		{
		}

		Message::Message(ByteArray && ba) : _bytes(std::move(ba))
		{
		}
		Message::~Message()
		{
		}
		void Message::setType(Type type)
		{
			if (_bytes.size() < MESSAGE_LEAST_SIZE)
			{
				_bytes.resize(MESSAGE_LEAST_SIZE);
			}
			*(reinterpret_cast<Type*>(&(_bytes[0]))) = type;
		}

		Message::Type Message::getType()
		{
			if (_bytes.size() < MESSAGE_LEAST_SIZE)
			{
				return Message::Type::UNKOWN;
			}
			return *(reinterpret_cast<const Type*>(_bytes.data()));
		}
		Message::IDType Message::getID()
		{
			if (_bytes.size() < MESSAGE_LEAST_SIZE)
			{
				return MESSAGE_ID_INVALID;
			}
			return *(reinterpret_cast<const IDType*>(_bytes.data() + sizeof(Message::Type)));
		}
		void Message::setID(IDType id)
		{
			if (_bytes.size() < MESSAGE_LEAST_SIZE)
			{
				_bytes.resize(MESSAGE_LEAST_SIZE);
			}
			*(reinterpret_cast<IDType*>(&(_bytes[0]) + sizeof(Message::Type))) = id;
		}
		const char * Message::getBytes()
		{
			if (_bytes.size() < MESSAGE_LEAST_SIZE)
			{
				_bytes.resize(0);
			}
			else
			{
				serialize();
			}
				return _bytes.data();
		}
		void Message::setRawBytes(ByteArray ba)
		{
			_bytes = std::move(ba);
		}
		void Message::initialze()
		{
			_bytes.reserve(MESSAGE_LEAST_SIZE + bytesCount());
			_bytes.resize(MESSAGE_LEAST_SIZE);
			setType(Message::Type::UNKOWN);
			setID(MESSAGE_ID_INVALID);
		}
	}
}