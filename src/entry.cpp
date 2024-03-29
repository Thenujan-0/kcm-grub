/*******************************************************************************
 * Copyright (C) 2008-2013 Konstantinos Smanis <konstantinos.smanis@gmail.com> *
 *                                                                             *
 * This program is free software: you can redistribute it and/or modify it     *
 * under the terms of the GNU General Public License as published by the Free  *
 * Software Foundation, either version 3 of the License, or (at your option)   *
 * any later version.                                                          *
 *                                                                             *
 * This program is distributed in the hope that it will be useful, but WITHOUT *
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for    *
 * more details.                                                               *
 *                                                                             *
 * You should have received a copy of the GNU General Public License along     *
 * with this program. If not, see <http://www.gnu.org/licenses/>.              *
 *******************************************************************************/

//Own
#include "entry.h"

//Project
#include "common.h"

// Qt
#include <QDebug>

Entry::Entry(const QString &strTitle, int numTitle, Entry::Type type, int level)
{
    m_title.str = strTitle;
    m_title.num = numTitle;
    m_type = type;
    m_level = level;
}

Entry *Entry::operator=(const Entry &rhs)
{
    Entry *entry = new Entry(rhs.title().str, rhs.title().num, rhs.type(), rhs.level());
    entry->setAncestors(rhs.ancestors());
    qWarning("equal operator called on Entry");
    return entry;
}

Entry::~Entry()
{
}

Entry::Title Entry::title() const
{
    return m_title;
}
QString Entry::prettyTitle() const
{
    return unquoteWord(m_title.str);
}
QString Entry::fullTitle() const
{
    QString fullTitle;
    for (const Entry::Title &ancestor : qAsConst(m_ancestors)) {
        fullTitle += unquoteWord(ancestor.str) += QLatin1Char('>');
    }
    return fullTitle + unquoteWord(m_title.str);
}
QString Entry::compactTitle() const
{
    if (m_ancestors.count() > 0) {
        return QString("    ") + unquoteWord(m_title.str);
    }

    return unquoteWord(m_title.str);
}
QString Entry::fullNumTitle() const
{
    QString fullNumTitle;
    for (const Entry::Title &ancestor : qAsConst(m_ancestors)) {
        fullNumTitle += QString::number(ancestor.num) += QLatin1Char('>');
    }
    return fullNumTitle + QString::number(m_title.num);
}
Entry::Type Entry::type() const
{
    return m_type;
}
int Entry::level() const
{
    return m_level;
}
QList<Entry::Title> Entry::ancestors() const
{
    return m_ancestors;
}
QString Entry::kernel() const
{
    return m_kernel;
}

void Entry::setTitle(const Entry::Title &title)
{
    m_title = title;
}
void Entry::setTitle(const QString &strTitle, int numTitle)
{
    m_title.str = strTitle;
    m_title.num = numTitle;
}
void Entry::setType(Entry::Type type)
{
    m_type = type;
}
void Entry::setLevel(int level)
{
    m_level = level;
}
void Entry::setAncestors(const QList<Entry::Title> &ancestors)
{
    m_ancestors = ancestors;
}
void Entry::setKernel(const QString &kernel)
{
    m_kernel = kernel;
}

void Entry::clear()
{
    m_title.str.clear();
    m_title.num = -1;
    m_type = Entry::Invalid;
    m_level = -1;
    m_ancestors.clear();
    m_kernel.clear();
}
