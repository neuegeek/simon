//
// C++ Implementation: wordlistmanager
//
// Description:
//
//
// Author: Peter Grasch <bedahr@gmx.net>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "wordlistmanager.h"
#include "logger.h"
#include <QObject>
#include <QList>
#include <QFile>
#include <QByteArray>
#include <QtGlobal>
#include <QTextStream>
#include "modelmanager.h"
#include "settings.h"
#include <QMessageBox>

WordListManager* WordListManager::instance;

/**
 * @brief Constructor
 *
 * Reads the vocab and initializes the wordlist (member) with it
 *
 * @author Peter Grasch
 * @param QString path
 * Sets the path (member) to the given string
 */
WordListManager::WordListManager () : QThread()
{
	isTemp = false;
	connect(this, SIGNAL(wordListCouldntBeLoaded()), this, SLOT(complainAboutPaths()));
	connect(this, SIGNAL(shadowListCouldntBeLoaded()), this, SLOT(complainAboutPaths()));
	connect(this, SIGNAL(tempWarning()), this, SLOT(warnAboutTempWordList()));

	wordListLock.lock();
	mainDirty = false;
	this->wordlist = readWordList ( Settings::getS("Model/PathToLexicon"), Settings::getS("Model/PathToVocab"), Settings::getS("Model/PathToPrompts"), this->activeTerminals );
	if (!wordlist) {
		this->wordlist = new WordList();
		emit wordListCouldntBeLoaded();
	}
	
	wordListLock.unlock();
	start(QThread::IdlePriority);
}


/**
 * \brief Returns the whole list of words using the specified terminal
 * @param terminal The terminal to look for
 * @param includeShadow Should we also look in the shadowdict?
 * @return The wordlist
 */
WordList* WordListManager::getWordsByTerminal(QString terminal, bool includeShadow)
{
	wordListLock.lock();
	WordList *list = getWordList();
	WordList *out = new WordList();
	for (int i=0; i < list->count(); i++)
	{
		if (list->at(i).getTerminal()==terminal)
			out->append(list->at(i));
	}
	wordListLock.unlock();
	if (!includeShadow) return out;
	
	shadowLock.lock();
	list = getShadowList();
	for (int i=0; i < list->count(); i++)
	{
		if (list->at(i).getTerminal()==terminal)
			out->append(list->at(i));
	}
	shadowLock.unlock();
	
	return out;
}



/**
 * \brief Warns about changing a temporary wordlist (when the path is not correctly configured)
 * \author Peter Grasch
 */
void WordListManager::warnAboutTempWordList()
{
	QMessageBox::warning(0, tr("Tempor�re �nderungen"), tr("Sie ver�ndern eine tempor�re Wordliste.\n\nDie �nderungen werden nicht gespeichert. Bitte konfigurieren Sie einen korrekten Pfad in den Einstellungen und starten Sie simon neu um eine dauerhafte Wortliste zu benutzen."));
}

void WordListManager::complainAboutPaths()
{
	QMessageBox::critical(0, tr("Lesefehler"), tr("Konnte ben�tigte Dateien f�r die Wortliste nicht einlesen. Bitte �berpr�fen Sie die Pfade zu den Vocab- und Lexikon-Dateien und stellen Sie sicher das sie die n�tigen Leserechte haben.\n\nDie Wortliste wird leer im RAM erstellt. �nderungen werden NICHT dauerhaft gespeichert."));
}

WordListManager* WordListManager::getInstance()
{
	if (!instance)
	{
		instance = new WordListManager();
	}
	return instance;
}

/**
 * \brief Starts the importing of the shadow-Model in a new thread
 * \author Peter Grasch
 */
void WordListManager::run()
{
	shadowLock.lock();
	shadowDirty = false;
	this->shadowList = readWordList(Settings::getS("Model/PathToShadowLexicon"), Settings::getS("Model/PathToShadowVocab"), Settings::getS("Model/PathToPrompts"), this->shadowTerminals, true);
	if (!shadowList)
	{
		this->shadowList = new WordList();
		emit shadowListCouldntBeLoaded();
	}
	shadowLock.unlock();
}


/**
 * \brief Sorts the given List of words
 * \author Peter Grasch
 * \param WordList* list
 * The list of words to sort
 * \return WordList*
 * The sorted list of words
 */
WordList* WordListManager::sortList(WordList* list)
{
	Logger::log(QObject::tr("[INF] Sortiere eine Liste mit %1 W�rtern").arg(list->count()));
	qSort(list->begin(), list->end());
	return list;
}


WordList* WordListManager::getShadowList()
{
	//if the thread is still running we are obviously not ready to give out the shadowdict
	//wait till we are finished
	if (isRunning())
	{
//FIXME: We can't show this directly!
//	If this method is called by a different thread (this WILL happen),
//	we would create the simonInfo object (which contains gui-widgets) in that
//	threads context
//	This will result in an async-error from xlib (you can't have different threads painting the gui)
//	and therefore result in a crash;
//	
// 		SimonInfo::showMessage(tr("Bitte warten Sie w�hrend das Schatten-W�rterbuch l�dt..."),1000);
// 		QCoreApplication::processEvents();
		//this is actually bug-using as we just set any kind of timeout (the gui thread will be
		//blocked until the importing is done anyway...
		wait();
	}
	
	return shadowList;
}

/**
 * \brief Saves the current WordList (member)
 * \author Peter Grasch
 * \param QString filename
 * If not provided we default on the filename we used to open the file
 * \warning If both lists are changed, we will ONLY emit the wordlistChanged signal
 * \return bool
 * Saving successful?
 */
bool WordListManager::save ( QString lexiconFilename, QString vocabFilename,
			     QString shadowLexiconFilename, QString shadowVocabFilename )
{
	bool wlistChanged = false, slistChanged = false;
	if (isTemp){
		emit tempWarning();
	}

	Logger::log(QObject::tr("[INF] Speichere W�rterliste"));
	
	//save wordlist
	wordListLock.lock();
	if (mainDirty)	//we changed the wordlist
	{
		if (lexiconFilename.isEmpty()) lexiconFilename = Settings::getS("Model/PathToLexicon");
		if (vocabFilename.isEmpty()) vocabFilename = Settings::getS("Model/PathToVocab");
		saveWordList(this->getWordList(), lexiconFilename, vocabFilename);
		wlistChanged = true;
		mainDirty=false;
	}
	wordListLock.unlock();
	
	//shadowlist
	shadowLock.lock();
	if (shadowDirty) //we changed the shadowDict
	{
		if (shadowLexiconFilename.isEmpty()) shadowLexiconFilename = Settings::getS("Model/PathToShadowLexicon");
		if (shadowVocabFilename.isEmpty()) shadowVocabFilename = Settings::getS("Model/PathToShadowVocab");
		saveWordList(this->getShadowList(), shadowLexiconFilename, shadowVocabFilename);
		slistChanged = true;
		shadowDirty=false;
	}
	shadowLock.unlock();
	
	if (wlistChanged) emit wordlistChanged();
	else if (slistChanged) emit shadowListChanged();
	return true;
}


/**
 * \brief Saves the given wordlist to the given files
 * \author Peter Grasch
 * @param list The list to save
 * @param lexiconFilename The lexicon to write to
 * @param vocabFilename The vocabfile to write to
 * @return Success
 */
bool WordListManager::saveWordList(WordList *list, QString lexiconFilename, QString vocabFilename)
{
	Logger::log(QObject::tr("[INF] �ffnen der Ausgabedatei: %1").arg(lexiconFilename));
	
	QFile *outfile = new QFile(lexiconFilename);
	if (!outfile->open(QIODevice::WriteOnly)) {
		Logger::log(QObject::tr("[ERR] Fehler beim �ffnen der Ausgabedatei %1").arg(lexiconFilename));
		outfile->deleteLater();
		return false;
	}
	QTextStream outstream(outfile);
	//TODO Test encoding
// 	outstream.setCodec("ISO 8859-15");


	QFile *vocabFile = new QFile(vocabFilename);
	if (!vocabFile->open(QIODevice::WriteOnly)) {
		Logger::log(QObject::tr("[ERR] Fehler beim �ffnen der Ausgabedatei %1").arg(vocabFilename));
		outfile->close();
		outfile->deleteLater();
		vocabFile->deleteLater();
		return false;
	}
	QTextStream vocab(vocabFile);
	//TODO Test encoding
// 	vocab.setCodec("ISO 8859-15");

	//print internal sentence structure
	// 	% NS_B
	// 	<s>	sil
	// 
	// 	% NS_E
	// 	</s>	sil
	vocab << "% NS_B\n<s>\tsil\n";
	vocab << "% NS_E\n</s>\tsil\n";

	QHash<QString /*terminalName*/, Word /*words*/> vocabulary;
	
	int i=0;

	bool foundSentEnd=false;
	int sentEndIndex = getWordIndex(list, foundSentEnd, "SENT-END", "sil");
	if (!foundSentEnd)
	{
		list->insert(sentEndIndex, Word("SENT-START", "sil", "deleteme"));
		list->insert(sentEndIndex, Word("SENT-END", "sil", "deleteme"));
	}
	//write lexicon
	int count = list->count();
	
	QStringList distinctTerminals;
	while (i<count)
	{
		Word w = list->at(i);
		QString wordStr = w.getWord();
		//TODO: Test naming
		QString upperWord = wordStr.toUpper();
		QString wordPron = w.getPronunciation();
		QString wordTerm = w.getTerminal();

		outstream << upperWord /*wordStr*/ << "\t\t[" << wordStr << "]\t\t" <<
				wordPron << "\n";

		vocabulary.insertMulti(wordTerm, w);
		if (!distinctTerminals.contains(wordTerm)) distinctTerminals.append(wordTerm);
		i++;
	}
	outfile->close();
	outfile->deleteLater();

	distinctTerminals.removeAll("deleteme");
	
	for (int i=0; i < distinctTerminals.count(); i++)
	{
		vocab << "% " << distinctTerminals[i] << "\n";
		WordList currentWordList = vocabulary.values(distinctTerminals[i]);
		int wordCount = currentWordList.count();
		for (int j=0; j < wordCount; j++)
		{
			Word w = currentWordList.at(j);
			vocab << w.getWord() << "\t" << w.getPronunciation() << "\n";
		}
	}

	Logger::log(QObject::tr("[INF] Schie�en der Ausgabedatei"));
	vocabFile->close();
	vocabFile->deleteLater();
	return true;
}

/**
 * \brief Binary search for finding the word
 * @param list The list to search
 * @param found Did we find something?
 * @param word The name of the word
 * @param pronunciation SAMPA pronunciation
 * @param terminal The terminal of the word
 * @note This is incredible fast :)
 * @return The index of the found word this is set to the nearest hit if not found (see parameter: found)
 */
int WordListManager::getWordIndex(WordList *list, bool &found, QString word, QString pronunciation, QString terminal)
{
	if (!list || (list->count()==0))
	{
		found = false;
		return -1;
	}
	word = word.toUpper();
	
	int currentSearchStart = list->count()/2; //make use of integer division
	//if count() == 1, currentSearchStart = 0,5 = 0 instead of 1 when using round
	//(which would be out of bounds)
	
	int currentMinValue = 0;
	int currentMaxValue = list->count();
	Word *currentWord;
	QString currentWordName, currentWordPronunciation, currentWordTerminal;
	int modificator=0;
	while (true)
	{
		currentWord = (Word*) &(list->at(currentSearchStart));
		currentWordName = currentWord->getWord().toUpper();
		currentWordPronunciation = currentWord->getPronunciation();
		currentWordTerminal = currentWord->getTerminal();
		
		if ((currentWordName==word)
			&& ((pronunciation.isEmpty() || currentWordPronunciation == pronunciation)
			&& (terminal.isEmpty() || currentWordTerminal == terminal)))
		{
			//we found the exact word
			found = true;
			return currentSearchStart;
		} else if ((currentWordName < word) || 
			((currentWordName == word) && ((!pronunciation.isEmpty() && currentWordPronunciation < pronunciation)
			|| (!terminal.isEmpty() && currentWordTerminal < terminal))))
		{
			currentMinValue = currentSearchStart;
			modificator = (currentMaxValue - currentMinValue)/2;
		} else if ((currentWordName > word) || 
			((currentWordName == word) && ((!pronunciation.isEmpty() && currentWordPronunciation > pronunciation)
			|| (!terminal.isEmpty() && currentWordTerminal > terminal))))
		{
			currentMaxValue = currentSearchStart;
			modificator = (currentMaxValue - currentMinValue)/(-2);
		}
		
		
		if (modificator == 0) {
			//stagnating search
			//do a incremental search over the left over items
			int i=currentMinValue;
			while ((i < currentMaxValue) && ((currentWordName < word) || 
						     ((currentWordName == word) && ((!pronunciation.isEmpty() && currentWordPronunciation < pronunciation)
						     || (!terminal.isEmpty() && currentWordTerminal < terminal)))))
			{
				i++;
			}
			if ((list->at(i).getWord().toUpper()==word)
							&& ((pronunciation.isEmpty() || list->at(i).getPronunciation() == pronunciation)
							&& (terminal.isEmpty() || list->at(i).getTerminal() == terminal)))
			{
				found = true;
			} else found = false;
			return i;
		}
		currentSearchStart += modificator;
	}
	
	found = false;
	return currentSearchStart;
}

/**
 * \brief Inits the lists to empty lists
 * \author Peter Grasch
 */
void WordListManager::safelyInit()
{
	this->wordlist = new WordList();
	this->shadowList = new WordList();
	isTemp = true;
}

/**
 * @brief Reads the Vocab and returns the read Data
 *
 * This method parses the given File and creates a WordList out of it
 *
 * @author Peter Grasch
 *
 * @param lexiconpath
 * Path to the lexicon
 * \param vocabpath
 * Path to the vocabulary
 * \param promptspath
 * Path to the prompts file
 * \param terminals
 * Pointer to the terminallist to fill
 * \param isShadowlist
 * Points out if this is a shadowlist - then we skip the recognition-check (it will always return 0)
 * @return  The parsed WordList
 */
WordList* WordListManager::readWordList ( QString lexiconpath, QString vocabpath, QString promptspath, QStringList &terminals, bool isShadowlist )
{
	Logger::log (QObject::tr("[INF] Lesen der W�rterliste bestehend aus "));
	Logger::log(QObject::tr("[INF] \t\tLexikon: %1,").arg(lexiconpath));
	Logger::log(QObject::tr("[INF] \t\tVocabular: %1,").arg(vocabpath));
	Logger::log(QObject::tr("[INF] \t\tPrompts: %1").arg(promptspath));

	WordList *wordlist = new WordList();
	//read the vocab
	TrainingManager *trainManager = TrainingManager::getInstance();

	PromptsTable *promptsTable = TrainingManager::getInstance()->getPrompts();	

	QFile vocab(vocabpath);
	if ( !vocab.open(QFile::ReadOnly) || !promptsTable) {
		return false;
	}
	QString line, term, word;
	QString pronunciation;
	int splitter;

	//skip NS_E and NS_B
	for (int i=0; (i < 4) && (!vocab.atEnd()); i++)
		vocab.readLine(1025);

	while (!vocab.atEnd())
	{
		line = QString(vocab.readLine(1024)).trimmed();
		if (!line.startsWith("% "))
		{
			//read the word
			splitter = line.indexOf("\t");
			word = line.left(splitter).trimmed();
			if (word.isEmpty()) continue;
			pronunciation = line.mid(splitter).trimmed();
			wordlist->append(Word(word, pronunciation, term, (isShadowlist) ? 0 : trainManager->getProbability(word.toUpper())));
		} else
		{
			//its a new terminal!
			term = line.mid(2).trimmed();
			//strip multiple definitions
			if (!terminals.contains(term)) terminals.append(term);
		}
	}
	wordlist = this->sortList(wordlist);
// 	Logger::log(QObject::tr("[INF] �ffnen des Lexikons von: %1").arg(lexiconpath));
// 	QFile *lexicon = new QFile ( lexiconpath );
// 	QFile vocab(vocabpath);
// 	if ( !lexicon->open ( QFile::ReadOnly ) || !vocab.open(QFile::ReadOnly) || !promptsTable) {
// 		lexicon->/*delete*/Later();
// 		return false;
// 	}
// 	lexicon->close();
// 	lexicon->deleteLater();

	Logger::log(QObject::tr("[INF] W�rterliste erstellt"));
	return wordlist;
}


WordList* WordListManager::removeDoubles(WordList *in)
{
	if (!in) return NULL;
	
	Logger::log(QObject::tr("[INF] Entfernen der doppelten Eintr�ge"));
	
	for (int i=0; i < in->count(); i++)
	{
		for (int j=i; j < in->count(); j++)
		{
			if (in->at(i).getWord() == in->at(j).getWord())
			{
				in->removeAt(j);
			}
		}
	}
	return in;
}

/**
 * \brief Gets the given word and returns a pointer to it (NULL if not found)
 * \note Uses the _fast_ binary search of getWordIndex()
 * @param word Name of the word
 * @param pronunciation Pronunciation of the word
 * @param terminal The terminal of the word
 * @param isShadowed If the word is shadowed (reference parameter)
 * @return The word (null if not found)
 */
Word* WordListManager::getWord(QString word, QString pronunciation, QString terminal, bool &isShadowed)
{
	Word *w=NULL;
	isShadowed = false;
	wordListLock.lock();
	bool found;
	WordList *wList = getWordList();
	int wIndex = getWordIndex(wList, found, word, pronunciation, terminal);
	if (found)
	{
		w = (Word*) &(wList->at(wIndex));
	}
	wordListLock.unlock();
	if (w)
		return w;

	isShadowed = true;
	shadowLock.lock();
	wList = getShadowList();
	wIndex = getWordIndex(wList, found, word, pronunciation, terminal);
	if (found)
	{
		w = (Word*) &(wList->at(wIndex));
	}
	shadowLock.unlock();

	return w;
}

/**
 * \brief Moves the given word to the shadowlist
 * \author Peter Grasch
 * \warning THEORETICALLY this code (we must lock the shadowlist AND the main wordlist) might yield to a deadlock
 * \note uses binary search to determine where exactly to insert the new (old) word
 * @param w The given word
 * @return success (i.e. the file is not found); (this may also be false due to an error when deleting the prompts for the word!)
 */
bool WordListManager::moveToShadow(Word *w)
{
	int i=0;
	if (!w) return false;
	if (!TrainingManager::getInstance()->deleteWord(w, true))
		return false;
	shadowLock.lock();
	wordListLock.lock();
	while (i < wordlist->count())
	{
		if (&(wordlist->at(i)) == w)
		{
			Word w = wordlist->takeAt(i);
			w.setProbability(0);
			bool found;
			int index = getWordIndex(shadowList, found, w.getWord(), w.getPronunciation(), w.getTerminal());
			if (found) {
				QMessageBox::information(0, tr("Duplikat erkannt"), tr("Dieses Wort existiert bereits im Schattenw�rterbuch.\n\nEs wird nicht nocheinmal eingef�gt"));
			} else
				shadowList->insert(index, w);
				
			shadowDirty = mainDirty = true;
			break;
		}
		i++;
	}
	wordListLock.unlock();
	shadowLock.unlock();
	return save();
}

bool WordListManager::deleteCompletely(Word *w, bool shadowed)
{
	if (!w) return false;
	//search for every sample that has the word in it and remove it
	//delete the entry in
	//	dict
	//	shadowdict
	//	voca
	//	prompts

	if (!shadowed)
	{
		TrainingManager::getInstance()->deleteWord(w); //if the word is shadowed we can't have any
		wordListLock.lock();
		WordList *main = getWordList();
		for (int i=0; i < main->count(); i++)
			if (&(main->at(i)) == w)
			{
				main->removeAt(i);
				mainDirty=true;
				break; //is unique
			}

		wordListLock.unlock();
		//training samples
	}
	else 
	{
		shadowLock.lock();
		WordList *shadow = getShadowList();
		for (int i=0; i < shadow->count(); i++)
			if (&(shadow->at(i)) == w)
			{
				shadow->removeAt(i);
				shadowDirty=true;
				break; //is unique
			}
		shadowLock.unlock();
	}
	
	return save();
}

/**
 * \brief Returns the Terminal of the name & pronunciation (it is pulled out of the wlist)
 * \author Peter Grasch
 * \param QString name
 * Name of the word
 * \param QString pronunciation
 * Pronunciation of the word
 * \param WordList *wlist
 * The list of words where we look for the Terminal
 * \return QString*
 * The found terminal (or NULL if none matched)
 */
QString* WordListManager::getTerminal(QString name, QString pronunciation, WordList *wlist)
{
	int i=0, wordcount = wlist->count();
	QString uppername = name.toUpper();
	
	while ((i < wordcount) && (wlist->at( i ).getWord().toUpper() != uppername) && 
			( wlist->at( i ).getPronunciation() != pronunciation))
	{ i++; }
	if ((i<wordcount) && (wlist->at(i).getWord().toUpper() == uppername) && 
			( wlist->at( i ).getPronunciation() == pronunciation))
	{
		//we found the word!
		return  new QString(((wlist->at( i ).getTerminal())));
	}
	// there was no result
	return NULL;
}

/**
 * \brief Returns a random wordname using this terminal
 * @param terminal The terminal of the word
 * @author Peter Grasch
 * @return Wordname
 */
QString WordListManager::getRandomWord(QString terminal)
{
	wordListLock.lock();
	WordList *main = getWordList();
	int start;
	int i;
	if (main->count() > 0)
	{
		start = qrand()%main->count();
		i=start;
		while (i<main->count())
		{
			if (main->at(i).getTerminal()==terminal)
			{
				wordListLock.unlock();
				return main->at(i).getWord();
			}
			i++;
		}
		//we didn't find anything till now
		//start again at the beginning and go till the point we started last time
		i=0;
		while (i<start)
		{
			if (main->at(i).getTerminal()==terminal)
			{
				wordListLock.unlock();
				return main->at(i).getWord();
			}
			i++;
		}
	}
	wordListLock.unlock();
		
	
	shadowLock.lock();
	//still didn't find anything?
	//move on to the shadowlist
	WordList *shadowList = getShadowList();
	
	if (shadowList->count() > 0)
	{
		start = qrand()%shadowList->count();
		i=start;
		while (i<shadowList->count())
		{
			if (shadowList->at(i).getTerminal()==terminal)
			{
				shadowLock.unlock();
				return shadowList->at(i).getWord();
			}
			i++;
		}
		//we didn't find anything till now
		//start again at the beginning and go till the point we started last time
		i=0;
		while (i<start)
		{
			if (shadowList->at(i).getTerminal()==terminal)
			{
				shadowLock.unlock();
				return shadowList->at(i).getWord();
			}
			i++;
		}
	}
	shadowLock.unlock();
	
	return terminal; //we couldn't find a word - return what we got
}
	
QStringList WordListManager::getTerminals(bool includeShadow)
{
	QStringList terminals;
	
	//main
	terminals << this->activeTerminals;
	if (includeShadow)
	{
		shadowLock.lock();
		getShadowList(); /* to wait if the thread is running */
		shadowLock.unlock();

		//merge the list
		for (int i=0; i < shadowTerminals.count(); i++)
			if (!terminals.contains(shadowTerminals.at(i)))
				terminals.append(shadowTerminals.at(i));
	}
	terminals.sort();


	return terminals;
}




/**
 * \brief Merges the lists
 * \warning The input pointer might become invalid after calling this function
 * @param a List a
 * @param b List b
 * @return The merged list
 */
WordList* WordListManager::mergeLists(WordList *a, WordList *b, bool keepDoubles)
{
	if (!a) return b;
	if (!b) return a;
	

	WordList *target;
	WordList *source;
	
	if (a->count() > b->count()) // a bigger than b
	{
		target = a;
		source = b;
	} else {
		target = b;
		source = a;
	}
	
	int wCount = source->count();
	for (int i=0; i < wCount; i++)
	{
		bool found;
		Word currentWord = source->at(i);
		int index = getWordIndex(target, found, currentWord.getWord(), currentWord.getPronunciation(),
					 currentWord.getTerminal());
		if (keepDoubles || !found)
			target->insert(index, currentWord);
	}
	
	delete source;
	return target;
}


WordList* WordListManager::getWords(QString word, bool includeShadow, bool fuzzy, bool keepDoubles)
{
	WordList *out;
	
	out = getMainstreamWords(word, fuzzy);
	
	if (!includeShadow) return out;

	return this->mergeLists(getShadowedWords(word, fuzzy), out, keepDoubles);
}


WordList* WordListManager::searchForWords(WordList *list, QString word, bool fuzzy)
{
	bool found;
	WordList *out = new WordList();
	if (!list) return out;
	
	if (!fuzzy)		// great! we can perform a binary search
	{
		int indexOfWord = getWordIndex(list, found, word);
		if (!found) return out;
		
		//go up and down around the found index and add all matching words
		int i=indexOfWord;
		while ((i > 0) && (list->at(i).getWord().toUpper() == word.toUpper()))
		{
			out->append(list->at(i));
			i--;
		}
		i=indexOfWord+1;
		while ((i < list->count()) && (list->at(i).getWord().toUpper() == word.toUpper()))
		{
			out->append(list->at(i));
			i++;
		}
	} else { //nope - incremental only :(
		int i=0;
		while(i < list->count())
		{
			if (list->at(i).getWord().contains(word, Qt::CaseInsensitive))
			{
				out->append(list->at(i));
			}
			i++;
		}
	}
	
	return out;
}

WordList* WordListManager::getMainstreamWords(QString word, bool fuzzy)
{
	wordListLock.lock();
	WordList* found = searchForWords(getWordList(), word, fuzzy);
	wordListLock.unlock();
	return found;
}

WordList* WordListManager::getShadowedWords(QString word, bool fuzzy)
{
	shadowLock.lock();
	WordList* found = searchForWords(getShadowList(), word, fuzzy);
	shadowLock.unlock();
	return found;
}

void WordListManager::renameTerminal(QString from, QString to, bool includeShadow)
{
	if (to == from) return;
	int i=0;
	wordListLock.lock();
	WordList *main = getWordList();
	while (i < main->count())
	{
		if (main->at(i).getTerminal()==from)
		{
			//discard the const-qualifier
			Word w = ((Word) main->at(i));
			w.setTerminal(to);
			main->replace(i, w);
			mainDirty = true;
		}
		i++;
	}
	wordListLock.unlock();
	activeTerminals.replaceInStrings(QRegExp("^"+from+"$"), to);

	if (!includeShadow) return;

	//shadow
	i=0;
	shadowLock.lock();
	WordList *shadow = getShadowList();
	while (i < shadow->count())
	{
		if (shadow->at(i).getTerminal()==from)
		{
			Word w = ((Word) shadow->at(i));
			w.setTerminal(to);
			shadow->replace(i, w);
			shadowDirty = true;
		}
		i++;
	}
	shadowLock.unlock();
	shadowTerminals.replaceInStrings(QRegExp("^"+from+"$"), to);
}

/**
 * \brief Adds the words to the local wordlist
 * Inserts the words into the current list (alphabetically)
 * Tries to omit duplicates...
 * 
 * This actually doesn't use binary search to look up the correct position where to insert the new words,
 * but rather a "smart" linear search:
 *  * We know that both the wordlists are sorted and start from the previous insertion spot
 * This is in the common case (large new voc; small oldvoc;) even faster.
 * 
 * \author Peter Grasch
 * \param WordList *list
 * List of words to add (DANGER: this pointer may be invalid after calling this function!)
 */
void WordListManager::addWords(WordList *list, bool isSorted, bool shadow)
{
	if (!isSorted)
		list = sortList(list);

	WordList *target;
	if (shadow)
	{
		shadowLock.lock();
		 target = getShadowList();
	} else {
		wordListLock.lock();
		target = getWordList();
	}
		

	Logger::log(QObject::tr("[INF] Hinzuf�gen von %1 W�rtern in die W�rterliste").arg(list->count()));

	if (shadow)
	{
		for (int i=0; i < list->count(); i++)
		{
			if (!shadowTerminals.contains(list->at(i).getTerminal()))
				shadowTerminals<< list->at(i).getTerminal();
		}
	}else
		for (int i=0; i < list->count(); i++)
		{
			if (!activeTerminals.contains(list->at(i).getTerminal()))
				activeTerminals << list->at(i).getTerminal();
		}

	int i=0;
	WordList *main, *newList;

	if (list->count() < target->count())
	{
		main=target;
		newList = list;
	} else
	{
		main= list;
		newList = target;
	}
	int wordcount = main->count();
	
	while (newList->count() >0)
	{
		Word tmp=newList->at(0);
		while ((i<wordcount-1) && (main->at(i) < tmp))
		{ i++; }
		if (main->at(i) != tmp) //no doubles
		{
			main->insert(i, tmp);
			wordcount++;
		} else newList->removeAt(0); //removeAt the double
	}

	delete newList;

	if (shadow) {
		this->shadowList = main;
		shadowDirty=true;
		shadowLock.unlock();
	} else {
		this->wordlist = main;
		mainDirty=true;
		wordListLock.unlock();
	}

	Logger::log(QObject::tr("[INF] Die Wortliste beinhaltet jetzt %1 W�rter").arg(wordlist->count()));
	
	this->save();
}

/**
 * \brief Reads the vocab of the given file and parses it into the WordList which is returned
 * \author Peter Grasch
 * \param QString vocabpath
 * Path to the vocabs
 * \return WordList*
 * The list of parsed vocab
 */
WordList* WordListManager::readVocab(QString vocabpath)
{
	Logger::log(QObject::tr("[INF] Lese Vokabular von %1").arg(vocabpath));
	WordList *vocablist = new WordList();
	
	QFile *vocab = new QFile ( vocabpath );
	vocab->open ( QFile::ReadOnly );
	if ( !vocab->isReadable() ) return false;

	
	QString line;
	QString name;
	QString pronunciation;
	QString terminal;
	int foundPos;


	if (!vocab->atEnd()) 
	{
		line = vocab->readLine(1024);
	
		//skip till the next terminal definition
		while (!vocab->atEnd() && (!line.startsWith("% ")))
		{ line = vocab->readLine(1024);  }
		
		//set the terminal to this one if there is one (i.e. the file was not over)
		if (!vocab->atEnd())
			terminal = line.mid(2).trimmed();
	}
	


	while ( !vocab->atEnd() ) //for each line that was successfully read
	{
		line =vocab->readLine(1024);
		if (line.trimmed().isEmpty()) continue;
		
		if (line.startsWith("%")) 
		{
			//The Line is the Definition of the terminal
			terminal = line.mid(2).trimmed();
			continue;
		}
		
		//parsing the line
		foundPos = line.indexOf ( "\t" );
		name = line.left ( foundPos ).trimmed();
		pronunciation = line.mid ( foundPos ).trimmed();
		
		//creates and appends the word to the wordlist
		vocablist->append ( Word(name, pronunciation, terminal, 0 ) );
	}
	vocab->close();
	vocab->deleteLater();

	return vocablist;
}

/**
 * \brief Builds and returns the promptstable by parsing the file at the given path
 * \author Peter Grasch
 * \param QString promptspath
 * The path to the prompts file
 * \return PromptsTable*
 * The table
 */
PromptsTable* WordListManager::readPrompts(QString promptspath)
{
	Logger::log(QObject::tr("[INF] Parse Promptsdatei von %1").arg(promptspath));
	PromptsTable *promptsTable = new PromptsTable();
	
	QFile *prompts = new QFile ( promptspath );
	prompts->open ( QFile::ReadOnly );
	if ( !prompts->isReadable() ) return false;
	
	QString label;
	QString prompt;
	QString line;
	int labelend;
	while ( !prompts->atEnd() ) //for each line that was successfully read
	{
		line = prompts->readLine(1024);
		labelend = line.indexOf(" ");
		label = line.left(labelend);
		prompt = line.mid(labelend).trimmed();

		promptsTable->insert( label, prompt );
	}
	prompts->close();
	prompts->deleteLater();
	
	return promptsTable;
}

/**
 * \brief Destructor
 * \author Peter Grasch
 */
WordListManager::~WordListManager()
{
    delete wordlist;
    delete shadowList;
}
