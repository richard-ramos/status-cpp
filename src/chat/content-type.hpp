#pragma once

#include <QObject>

class ContentTypeClass
{
	Q_GADGET
public:
	explicit ContentTypeClass();

	enum Value
	{
		FetchMoreMessagesButton = -2,
		ChatIdentifier = -1,
		Unknown = 0,
		Message = 1,
		Sticker = 2,
		Status = 3,
		Emoji = 4,
		Transaction = 5,
		Group = 6,
		Image = 7,
		Audio = 8,
		Community = 9
	};

	Q_ENUM(Value)
};

typedef ContentTypeClass::Value ContentType;

Q_DECLARE_METATYPE(ContentType)
