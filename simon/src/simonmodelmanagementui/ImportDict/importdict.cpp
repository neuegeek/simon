/*
 *   Copyright (C) 2008 Peter Grasch <peter.grasch@bedahr.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "importdict.h"
#include <simonlogging/logger.h>
#include "bompdict.h"
#include "sphinxdict.h"
#include "lexicondict.h"
#include "plsdict.h"
#include "juliusvocabulary.h"
#include <QFile>
#include <KDebug>
#include <KLocalizedString>

/**
 * \brief Constructor
 * \author Peter Grasch
 */
ImportDict::ImportDict(QObject* parent) : QThread(parent),
type(0),
dict(0),
deleteFileWhenDone(false)
{
}


/**
 * \brief Parses the list of words of an existing dictionary
 * \author Peter Grasch
 */
void ImportDict::parseWordList(QString pathToDict, QString encoding, int type, bool deleteFileWhenDone)
{
  this->pathToDict = pathToDict;
  this->type = type;
  this->encoding = encoding;
  this->deleteFileWhenDone = deleteFileWhenDone;
  start(/*QThread::IdlePriority*/);
}


QList<Word*> ImportDict::getCurrentWordList()
{
  QList<Word*> realList = wordList;
  wordList.clear();
  return realList;
}


/**
 * \brief The main execution loop
 * Does the real work.
 * Uses the members pathToDict to get the location and writes the finished list of word to the member wordList
 * \author Peter Grasch
 */
void ImportDict::run()
{
  Logger::log(i18n("Opening Lexicon \"%1\"", pathToDict));
  emit status(i18n("Opening Lexicon..."));

  emit progress(10, 1000);
  delete dict;
  wordList.clear();
  
  switch (type) {
    case Dict::HadifixBOMP:
      dict = new BOMPDict();
      break;
    case Dict::HTKLexicon:
      dict = new LexiconDict();
      break;
    case Dict::PLS:
      dict = new PLSDict();
      break;
    case Dict::SPHINX:
      dict = new SPHINXDict();
      break;
    case Dict::JuliusVocabulary:
      dict = new JuliusVocabulary();
      break;
    default:
      emit failed();
      return;                                     //unknown type
  }

  connect(dict, SIGNAL(loaded()), this, SLOT(openingFinished()));
  connect(dict, SIGNAL(progress(int)), this, SLOT(loadProgress(int)));

  emit status(i18n("Processing Lexicon..."));

  dict->load(pathToDict, encoding);
  emit status(i18n("Creating List..."));
  QStringList words = dict->getWords();
  QStringList categories = dict->getCategories();
  QStringList pronunciations = dict->getPronuncations();
  kDebug() << "Deleting dict!";
  delete dict;
  dict=0;

  int wordCount = words.count();
  for (int i=0; i<wordCount; i++) {
    wordList.append( new Word(words.at(i),
      pronunciations.at(i),
      categories.at(i) ) );
    if ((i%1000) == 0)
      emit progress((int) ((((double) i)/((double)words.count())) *40+800), 1000);
  }
  words.clear();
  pronunciations.clear();
  categories.clear();

  if (type != Dict::HTKLexicon) {
    emit status(i18n("Sorting Dictionary..."));
    qSort(wordList.begin(), wordList.end(), isWordLessThan);
  }
  emit progress(1000, 1000);
  emit status(i18n("Storing Dictionary..."));
  
  Logger::log(i18np("%1 word from the lexicon \"%2\" imported", "%1 words from the lexicon \"%2\" imported", words.count(), pathToDict));

  if (deleteFileWhenDone) {
    Logger::log(i18n("Deleting Input-File"));
    QFile::remove(this->pathToDict);
  }
  emit successful();
}


/**
 * \brief Translates the progress of the loading of the file to the global importing progress
 * emits the progress(int) signal with the calculated prog.
 * \author Peter Grasch
 * \param int prog
 * The progress of the loading
 */
void ImportDict::loadProgress(int prog)
{
  if (prog == -1) {
    emit progress(1, 0);
    return;
  }

  int globalProg =(int)  ((((double)prog)/1000)*800+10);
  emit progress(globalProg, 1000);
}


/**
 * \brief Deletes the dict
 * \author Peter Grasch
 */
void ImportDict::deleteDict()
{
  if (isRunning()) terminate();
  if (wait(2000))
  if (dict) {
    dict->deleteLater();
    dict=0;
  }
}


/**
 * \brief Finished opening the file
 * Emits opened()
 * \author Peter Grasch
 */
void ImportDict::openingFinished()
{
  emit opened();
}


/**
 * \brief Destructor
 * \author Peter Grasch
 */
ImportDict::~ImportDict()
{
  if (dict) dict->deleteLater();
}
