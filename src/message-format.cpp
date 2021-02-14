#include "message-format.hpp"
#include "contacts-model.hpp"
#include "message.hpp"
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QStringBuilder>

using namespace Messages;
using namespace Messages::Format;

/*

// TODO: make this receive a contacts model
QString MessagesModel::mention(QString pubKey) {
	if(m_contacts.get(pubKey) != null){
		return "ABCABCABC" // FORMAT  username with same function as qml, and removing suffix
	} else {
		return Utils::generateAlias(pubKey);
	}
}*/

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
	case Mention: return "<a href=\"//" % value % "\" class=\"mention\">" % "ABCDEFG" % "</a>";
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
