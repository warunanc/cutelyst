/*
 * Copyright (C) 2013-2014 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "action_p.h"
#include "controller.h"
#include "context.h"
#include "common.h"

using namespace Cutelyst;

Action::Action(QObject *parent) : Component(parent)
    , d_ptr(new ActionPrivate)
{
}

Action::~Action()
{
    delete d_ptr;
}

Component::Modifiers Action::modifiers() const
{
    return Component::OnlyExecute;
}

void Action::setMethod(const QMetaMethod &method)
{
    Q_D(Action);
    d->method = method;
    if (method.returnType() == QMetaType::Bool) {
        d->evaluateBool = true;
    }

    if (method.parameterCount() == 2 && method.parameterType(1) == QMetaType::QStringList) {
        d->listSignature = true;
    }
}

void Action::setController(Controller *controller)
{
    Q_D(Action);
    d->controller = controller;
}

void Action::setupAction(const QVariantHash &args, Application *app)
{
    Q_D(Action);

    init(app, args);

    d->ns = args.value(QLatin1String("namespace")).toString();

    const auto attributes = args.value(QLatin1String("attributes")).value<QMap<QString, QString> >();
    d->attributes = attributes;

    const QString argsAttr = attributes.value(QLatin1String("Args"));
    if (!argsAttr.isEmpty()) {
        d->numberOfArgs = argsAttr.toInt();
    }

    const QString capturesAttr = attributes.value(QLatin1String("CaptureArgs"));
    if (!capturesAttr.isEmpty()) {
        d->numberOfCaptures = capturesAttr.toInt();
    }
}

QMap<QString, QString> Action::attributes() const
{
    Q_D(const Action);
    return d->attributes;
}

void Action::setAttributes(const QMap<QString, QString> &attributes)
{
    Q_D(Action);
    d->attributes = attributes;
}

QString Action::className() const
{
    Q_D(const Action);
    return QString::fromLatin1(d->controller->metaObject()->className());
}

Controller *Action::controller() const
{
    Q_D(const Action);
    return d->controller;
}

bool Action::dispatch(Context *c)
{
    return c->execute(this);
}

bool Action::match(int numberOfArgs) const
{
    Q_D(const Action);
    // If the number of args is -1 (not defined)
    // it will slurp all args so we don't care
    // about how many args was passed, otherwise
    // count them
    return d->numberOfArgs == -1 || d->numberOfArgs == numberOfArgs;
}

bool Action::matchCaptures(int numberOfCaptures) const
{
    Q_D(const Action);
    // If the number of capture args is -1 (not defined)
    // it will slurp all args so we don't care
    // about how many args was passed, otherwise
    // count them
    return d->numberOfCaptures == -1 || d->numberOfCaptures == numberOfCaptures;
}

QString Action::ns() const
{
    Q_D(const Action);
    return d->ns;
}

qint8 Action::numberOfArgs() const
{
    Q_D(const Action);
    return d->numberOfArgs;
}

qint8 Action::numberOfCaptures() const
{
    Q_D(const Action);
    return d->numberOfCaptures;
}

bool Action::doExecute(Context *c)
{
    Q_D(const Action);
    if (c->detached()) {
        return false;
    }

    bool ret;
    if (d->evaluateBool) {
        bool methodRet;

        if (d->listSignature) {
            ret = d->method.invoke(d->controller,
                                   Qt::DirectConnection,
                                   Q_RETURN_ARG(bool, methodRet),
                                   Q_ARG(Cutelyst::Context*, c),
                                   Q_ARG(QStringList,  c->request()->args()));
        } else {
            QStringList args = c->request()->args();
            // Fill the missing arguments
            args += d->emptyArgs;

            ret = d->method.invoke(d->controller,
                                   Qt::DirectConnection,
                                   Q_RETURN_ARG(bool, methodRet),
                                   Q_ARG(Cutelyst::Context*, c),
                                   Q_ARG(QString, args.at(0)),
                                   Q_ARG(QString, args.at(1)),
                                   Q_ARG(QString, args.at(2)),
                                   Q_ARG(QString, args.at(3)),
                                   Q_ARG(QString, args.at(4)),
                                   Q_ARG(QString, args.at(5)),
                                   Q_ARG(QString, args.at(6)),
                                   Q_ARG(QString, args.at(7)),
                                   Q_ARG(QString, args.at(8)));
        }

        if (ret) {
            c->setState(methodRet);
            return methodRet;
        }

        // The method failed to be called which means we should detach
        c->detach();
        c->setState(false);

        return false;
    } else {
        if (d->listSignature) {
            ret = d->method.invoke(d->controller,
                                   Qt::DirectConnection,
                                   Q_ARG(Cutelyst::Context*, c),
                                   Q_ARG(QStringList, c->request()->args()));
        } else {
            QStringList args = c->request()->args();
            // Fill the missing arguments
            args += d->emptyArgs;

            ret = d->method.invoke(d->controller,
                                   Qt::DirectConnection,
                                   Q_ARG(Cutelyst::Context*, c),
                                   Q_ARG(QString, args.at(0)),
                                   Q_ARG(QString, args.at(1)),
                                   Q_ARG(QString, args.at(2)),
                                   Q_ARG(QString, args.at(3)),
                                   Q_ARG(QString, args.at(4)),
                                   Q_ARG(QString, args.at(5)),
                                   Q_ARG(QString, args.at(6)),
                                   Q_ARG(QString, args.at(7)),
                                   Q_ARG(QString, args.at(8)));
        }
        c->setState(ret);
        return ret;
    }
}
