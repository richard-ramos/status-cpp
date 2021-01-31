#pragma once

#include <QObject>

class ChatTypeClass
{
    Q_GADGET
public:
    explicit ChatTypeClass();

    enum Value {
        Unknown,
        OneToOne,
        Public,
        PrivateGroupChat,
        Profile,
        Timeline,
        ComunityChat
    };
    
    Q_ENUM(Value)

};

typedef ChatTypeClass::Value ChatType;
