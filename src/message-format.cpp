#include "message-format.hpp"
#include "base58.h"
#include "constants.hpp"
#include "contact.hpp"
#include "contacts-model.hpp"
#include "message.hpp"
#include "settings.hpp"
#include "utils.hpp"
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QStringBuilder>
#include <iostream>
#include <string>

using namespace Messages;
using namespace Messages::Format;

QString mention(QString pubKey, ContactsModel* contactsModel)
{
	Contact* contact(contactsModel->get(pubKey));
	if(contact != nullptr)
	{
		if(contact->get_ensVerified() && contact->get_name() != "")
		{
			if(contact->get_name().endsWith(Constants::StatusDomain))
			{
				return contact->get_name().left(contact->get_name().length() - 14);
			}
			else
			{
				return contact->get_name();
			}
		}
		else if(contact->get_localNickname() != "")
		{
			return contact->get_localNickname();
		}
		else
		{
			return contact->get_alias();
		}
	}
	else
	{
		if(Settings::instance()->publicKey() == pubKey)
		{
			// TODO: check if ens name
		}
		return Utils::generateAlias(pubKey);
	}
}

QHash<QString, RenderInlineTypes> renderInlineMap{
	{"", Empty},
	{"code", Code},
	{"emph", Emph},
	{"strong", Strong},
	{"strong-emph", StrongEmph},
	{"link", Link},
	{"mention", Mention},
	{"status-tag", StatusTag},
	{"del", Del},
};

// See render - inline in status - react / src / status_im / ui / screens / chat / message / message.cljs
QString renderInline(const QJsonObject& elem, ContactsModel* contactsModel)
{
	QString value(elem["literal"].toString().toHtmlEscaped().replace("\r\n", "<br />").replace("\n", "<br />"));
	QString textType = elem["type"].toString();

	if(!renderInlineMap.contains(textType))
		return " " % value % " ";

	switch(renderInlineMap[textType])
	{
	case Empty: return value;
	case Code: return "<code>" % value % "</code>";
	case Emph: return "<em>" % value % "</em>";
	case Strong: return "<strong>" % value % "</strong>";
	case StrongEmph: return "<strong><em>" % value % "</em></strong>";
	case Link: return elem["destination"].toString();
	case Mention: return "<a href=\"//" % value % "\" class=\"mention\">" % mention(value, contactsModel) % "</a>";
	case StatusTag: return "<a href=\"#" % value % "\" class=\"status-tag\">#" % value % "</a>";
	case Del: return "<del>" % value % "</del>";
	}
	return "";
}

// See render - block in status - react / src / status_im / ui / screens / chat / message / message.cljs
// TODO: make this receive a contacts model and make the functio nstatic and in a message utils class

QString paragraph(const QJsonObject& p, ContactsModel* contactsModel)
{
	QStringList inlineResult;
	foreach(const QJsonValue& child, p["children"].toArray())
	{
		inlineResult << renderInline(child.toObject(), contactsModel);
	}

	return QStringLiteral("<p>") % inlineResult.join("") % QStringLiteral("</p>");
}

QString blockquote(const QJsonObject& p)
{
	return QStringLiteral("<table class=\"blockquote\">") % QStringLiteral("<tr>") %
		   QStringLiteral("<td class=\"quoteline\" valign=\"middle\"></td>") % QStringLiteral("<td>") %
		   p["literal"].toString().toHtmlEscaped().split("\n").join("<br/>") % QStringLiteral("</td>") % QStringLiteral("</tr>") %
		   QStringLiteral("</table>");
}

QString codeblock(const QJsonObject& p)
{
	return QStringLiteral("<code>") % p["literal"].toString().toHtmlEscaped() % QStringLiteral("</code>");
}

QHash<QString, RenderBlockTypes> renderBlockMap{{"paragraph", Paragraph}, {"blockquote", Blockquote}, {"codeblock", Codeblock}};

QString Messages::Format::renderBlock(Message* message, ContactsModel* contactsModel)
{
	QStringList result;
	foreach(const QJsonValue& pMsg, message->get_parsedText())
	{
		const QJsonObject p = pMsg.toObject();
		QString textType = p["type"].toString();

		if(!renderBlockMap.contains(textType))
			continue;

		switch(renderBlockMap[textType])
		{
		case RenderBlockTypes::Paragraph: result << paragraph(p, contactsModel); break;
		case RenderBlockTypes::Blockquote: result << blockquote(p); break;
		case RenderBlockTypes::Codeblock: result << codeblock(p); break;
		}
	}
	return result.join("");
}

QString Messages::Format::decodeSticker(Message* message)
{
	QString stickerHash = message->get_sticker_hash();

	if(stickerHash.left(2) != QStringLiteral("e3"))
	{
		qWarning() << "Could not decode sticker. It may still be valid, but requires a different codec to be used: " + message->get_sticker_hash();
		return "";
	}

	if(stickerHash.left(6) == QStringLiteral("e30170"))
	{
		stickerHash.remove(0, 6);
	}

	if(message->get_sticker_hash().left(8) == QStringLiteral("e3010170"))
	{
		stickerHash.remove(0, 8);
	}

	std::vector<unsigned char> vch;
	for(int i = 0; i < stickerHash.length(); i += 2)
	{
		QString byteString = stickerHash.mid(i, 2);
		unsigned char b = std::strtol(byteString.toUtf8().data(), NULL, 16);
		vch.push_back(b);
	}

	return QString::fromStdString(EncodeBase58(vch));
}