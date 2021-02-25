#pragma once

#include <QObject>

class MessageTypeClass
{
	Q_GADGET
public:
	explicit MessageTypeClass();

	enum Value
	{
		Unknown = 0,
		OneToOne,
		Public,
		PrivateGroupChat,
		SystemMessagePrivateGroup,
		CommunityChat
	};

	Q_ENUM(Value)
};

typedef MessageTypeClass::Value MessageType;
