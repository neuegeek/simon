/*
 *  Copyright (C) 2010 Peter Grasch <peter.grasch@bedahr.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2,
 *  or (at your option) any later version, as published by the Free
 *  Software Foundation
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this program; if not, write to the
 *  Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef SIMON_LATEXREPORTTEMPLATEENGINE_H_4002119636CC42C68FE07273F9000A73
#define SIMON_LATEXREPORTTEMPLATEENGINE_H_4002119636CC42C68FE07273F9000A73

#include "reporttemplateengine.h"
class LatexReportTemplateEngine : public ReportTemplateEngine
{

  private:
    QByteArray lineBreak();
    QByteArray escape(const QByteArray& in, bool extraSafety);
  public:
    QString fileType();
    bool storeFile(const QByteArray& output, const QString& outputFilename, const QStringList& associatedFiles);
};

#endif


