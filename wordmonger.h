#ifndef WORDMONGER_H
#define WORDMONGER_H

#include <QGraphicsBlurEffect>
#include <QDebug>
#include <QTimer>
#include <QElapsedTimer>
#include <QGridLayout>
#include <QMainWindow>
#include <QObject>
#include <QLineEdit>
#include <QWidget>

#include <set>
#include <vector>

#include "fixed_string.h"
#include "gaddag.h"

class QLineEdit;

class Question : public QWidget {
 public:
  Question(QWidget* parent = 0);
  Question(QWidget* parent = 0, int index = -1);
  void MarkAnswer(const QString& answer);
  bool IsAllSolved() const;
  void SetBlurRadius(int radius) {
    blur_effect->setBlurRadius(radius);
    setGraphicsEffect(blur_effect);
  }
 protected:
  void paintEvent(QPaintEvent* event);

 private:
  int index;
  std::set<int> solved_answer_indices;
  QGraphicsBlurEffect* blur_effect;
};

class WordStatusBar : public QWidget {
 public:
  WordStatusBar(QWidget* parent = 0);
  void SetTime(int time_millis) { this->time_millis = time_millis; }
  void SetFontSize(int font_size) { this->font_size = font_size; }
 protected:
  void paintEvent(QPaintEvent *event);

 private:
  int time_millis;
  int font_size;

};

class QuestionAndAnswer {
 public:
  QuestionAndAnswer(const QString& clue, const std::vector<QString>& answers);
  QString GetClue() const;
  const std::vector<QString>& GetAnswers() const {
    return answers;
  }

 private:
  QString clue;
  std::vector<QString> answers;
};

class Wordmonger : public QMainWindow {
  Q_OBJECT

 public:
  Wordmonger(QWidget* parent);
  static Wordmonger* get();

  const QuestionAndAnswer& QuestionAndAnswerAt(int i) const {
    return questions_and_answers[i];
  }
  void CheckIfQuizFinished();
  bool QuizFinished() const { return quiz_finished; }
  bool IsTwl(QString word) const {
    return twl.count(word) > 0;
  }

  ~Wordmonger();

 public slots:
  void textChangedSlot(QString text) {
    // qInfo() << "text: " << text;
    QString uppercase_text = text.toUpper();
    if (text == uppercase_text) return;

    const int cursor_position = answer_line_edit->cursorPosition();
    answer_line_edit->setText(uppercase_text);
    answer_line_edit->setCursorPosition(cursor_position);

    auto it = answer_map.find(uppercase_text);
    if (it != answer_map.end()) {
      for (Question* question : it->second) {
        if (quiz_finished) {
          qInfo() << "typed " << uppercase_text << " too late";
        } else {
          question->MarkAnswer(uppercase_text);
        }
      }
      answer_line_edit->setText("");
    }
  }

 protected:
  void resizeEvent(QResizeEvent* event) override;
  void timerEvent(QTimerEvent *event) override;

 private:
  void CreateWidgets();
  void ChooseWords();
  void StartTimer();
  void LoadSingleAnagramWords();
  std::vector<QuestionAndAnswer> questions_and_answers;
  void AddQuestions();

  QGridLayout* questions_layout;
  std::vector<Question*> questions;
  std::map<QString, std::vector<Question*>> answer_map;

  void LoadDictionaries();
  void LoadDictionary(const QString& path, std::set<QString>* dict);
  void LoadGaddag(const QString& path);
  void TestGaddag();
  void Anagram(const unsigned char* node, int* counts,
               uint32_t rack_bits, WordString* prefix,
               std::vector<WordString>* anagrams);

  std::set<QString> twl;
  std::set<QString> csw;
  char* gaddag_bytes_;
  Gaddag* gaddag_;

  QLineEdit* answer_line_edit;
  WordStatusBar* word_status_bar;

  int timer_millis;
  bool time_expired;
  bool quiz_finished;
  QElapsedTimer elapsed_timer;
  QBasicTimer timer;

  static Wordmonger* self;
};

#endif  // WORDMONGER_H
