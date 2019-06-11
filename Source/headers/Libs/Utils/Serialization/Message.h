#pragma once

#include "serialization.h"

namespace Messaging
{
namespace IPC
{
	class Message
	{
	public:
		typedef int IDType;
		
		enum Type
		{
			STATUS_REGISTER = 0,
			STATUS_UNREGISTER,
			STATUS_SET,
			ACTION_REQUEST,
			ABORT_REQUEST,
			ACTION_RESULT,
			UNKOWN
		};

		static bool isValid(IDType id);
		Message();
		Message(const ByteArray& ba);
		Message(ByteArray&& ba);
		Message(const Message&) = default;
		Message(Message&&) = default;
		Message& operator=(const Message&) = default;
		Message& operator=(Message&&) = default;
		virtual ~Message();

		void setType(Type type);
		Type getType();
		IDType getID();
		void setID(IDType id);
		const char* getBytes();
		void setRawBytes(ByteArray ba);
		void initialze();
	protected:
		virtual size_t serialize() = 0;
		virtual size_t bytesCount() = 0;
		ByteArray _bytes;
	};
}
}