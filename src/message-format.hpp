#pragma once

#include "contacts-model.hpp"
#include "message.hpp"
#include <QHash>
#include <QString>

namespace Messages
{
namespace Format
{

enum RenderInlineTypes
{
	Empty,
	Code,
	Emph,
	Strong,
	StrongEmph,
	Link,
	Mention,
	StatusTag,
	Del
};

enum RenderBlockTypes
{
	Paragraph,
	Blockquote,
	Codeblock
};

QString renderBlock(Message* message, ContactsModel* contactsModel);

} // namespace Format
} // namespace Messages