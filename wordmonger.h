#ifndef WORDMONGER_H
#define WORDMONGER_H

#include <QGraphicsBlurEffect>
#include <QDebug>
#include <QLabel>
#include <QTimer>
#include <QElapsedTimer>
#include <QGridLayout>
#include <QPushButton>
#include <QMainWindow>
#include <QObject>
#include <QLineEdit>
#include <QWidget>

#include <set>
#include <vector>

#include "fixed_string.h"
#include "gaddag.h"

class QLineEdit;
class QuizPushButton;

class QuizChooser : public QWidget {
 public:
  QuizChooser(QWidget* parent = 0);

 protected:
  void resizeEvent(QResizeEvent *event);
};

enum ChooserType { BUTTONS, LINE_EDITS };

enum ButtonType { MULTIPLE_IN_ROW, ONE_IN_ROW };

class ChooserButtonRow : public QWidget {
 public:
  ChooserButtonRow(QWidget* parent = 0,
                   const QString& label_text = "",
                   ChooserType chooser_type = BUTTONS,
                   ButtonType button_type = MULTIPLE_IN_ROW);
  void AddButton(const QString& label_text, const QString& id);
  void AddLabelledLineEdit(const QString& label, int value,
                           bool editable, QLineEdit** line_edit_ptr = nullptr,
                           int width = 36, int min = 1, int max = 99);
  void AddLabel(const QString& label_text);
  void AddLineEditsStretch();
  ButtonType ButtonType() const { return button_type_; }
  QList<QuizPushButton*> buttons_list;

 protected:
  void resizeEvent(QResizeEvent *event);

 private:
  QVBoxLayout* layout;
  QList<QWidget*> line_edits_list;
  QWidget* buttons;
  enum ButtonType button_type_;
  QHBoxLayout* buttons_layout;
  QWidget* line_edits;
  QHBoxLayout* line_edits_layout;
  QLabel* label;
};

class LabelledLineEdit : public QWidget {
 public:
  LabelledLineEdit(QWidget* parent = 0,
                   const QString& label_text = "",
                   int value = 0,
                   bool editable = true,
                   QLineEdit** line_edit_ptr = nullptr,
                   int width = 36, int min = 1, int max = 99);
private:
  QVBoxLayout* layout;
  QLabel* label;
  QLineEdit* line_edit;
};

class Preview : public QWidget {
 public:
  Preview(QWidget* parent = 0);

 protected:
  void resizeEvent(QResizeEvent* event);
};

class DetailChooser : public QWidget {
 public:
  DetailChooser(QWidget* parent = 0);

 protected:
  void resizeEvent(QResizeEvent* event);
};

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

class QuizPushButton : public QPushButton {
 public:
  QuizPushButton(ChooserButtonRow* parent_row, QWidget* parent = 0);
  const ChooserButtonRow* ParentRow() const { return parent_row_; }
 private:
  const ChooserButtonRow* parent_row_;
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
  QString FontName() const { return font_name; }
  QFont::Weight FontWeight() const { return font_weight; }
  QVBoxLayout* QuizChooserLayout() {return quiz_chooser_layout; }

  ~Wordmonger();

 public slots:
  void TogglePauseSlot() { paused ? UnpauseTimer() : PauseTimer(); }

  void ToggleFullscreenSlot() {
    isMaximized() ? showNormal() : showMaximized();
  }

  void TextChangedSlot(QString text) {
    //qInfo() << "TextChangedSlot(), text: " << text;
    QString uppercase_text = text.toUpper();
    if (text == uppercase_text) return;

    const int cursor_position = answer_line_edit->cursorPosition();
    answer_line_edit->setText(uppercase_text);
    answer_line_edit->setCursorPosition(cursor_position);

    if (paused) {
      qInfo() << "The timer is paused.";
      return;
    }
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

  void RowsChangedSlot(QString text) {
    qInfo() << "RowsChangedSlot(), text: " << text;
    if (!text.isEmpty() && cols_line_edit != nullptr &&
        !cols_line_edit->text().isEmpty()) {
      num_words_line_edit->setText(QString::number(
                                     RequestedRows() * RequestedCols()));
    }
  }

  void ColsChangedSlot(QString text) {
    qInfo() << "ColsChangedSlot(), text: " << text;
    if (!text.isEmpty() && rows_line_edit != nullptr &&
        !rows_line_edit->text().isEmpty()) {
      num_words_line_edit->setText(QString::number(
                                     RequestedRows() * RequestedCols()));
    }
  }

  void ButtonReleasedSlot() {
    const QuizPushButton* sender_button = (const QuizPushButton*)sender();
    qInfo() << "sender: " << sender_button->text();
    if (!sender_button->isChecked()) {
      qInfo() << "sender is not checked";
      return;
    }
    const ChooserButtonRow* parent_row =
        (const ChooserButtonRow*)sender_button->ParentRow();
    const ButtonType button_type = parent_row->ButtonType();
    for (ChooserButtonRow* row : {word_length, word_builder, blanks}) {
      for (QPushButton* button : row->buttons_list) {
        qInfo() << "button: " << button->text();
        if (row != parent_row) {
          qInfo() << "in different row, setChecked(false)";
          button->setChecked(false);
          continue;
        }
        if (button_type == MULTIPLE_IN_ROW) {
          qInfo() << "in same row but allowing multiple";
          continue;
        }
        if (button == sender()) {
          qInfo() << "is sender";
          continue;
        }
        qInfo() << "setChecked(false)";
        button->setChecked(false);
      }
    }
  }

   protected:
    void resizeEvent(QResizeEvent * event) override;
    void timerEvent(QTimerEvent * event) override;

   private:
    void CreateCentralWidgetAndLayout();
    void CreateQuizChoiceWidgets();
    void CreateGridQuizWidgets();
    void ChooseWords();
    void DrawRacks();
    void StartTimer();
    void PauseTimer();
    void UnpauseTimer();
    void LoadSingleAnagramWords();
    std::vector<QuestionAndAnswer> questions_and_answers;
    void AddQuestions();
    int RequestedRows();
    int RequestedCols();

    QWidget* central_widget;
    QVBoxLayout* central_layout;

    QuizChooser* quiz_chooser;
    QVBoxLayout* quiz_chooser_layout;
    ChooserButtonRow* word_length;
    ChooserButtonRow* word_builder;
    ChooserButtonRow* blanks;
    ChooserButtonRow* num_solutions;
    ChooserButtonRow* rows_and_columns;
    ChooserButtonRow* ordering;
    ChooserButtonRow* quiz_time;

    Preview* quiz_preview;
    DetailChooser* detail_chooser;
    QGridLayout* choosers_layout;

    QGridLayout* questions_layout;
    std::vector<Question*> questions;
    std::map<QString, std::vector<Question*>> answer_map;

    QMenuBar* menu_bar;
    QAction* pause_action;
    QAction* fullscreen_action;
    QMenu* quiz_menu;
    void CreateMenus();

    void LoadDictionaries();
    void LoadDictionary(const QString& path, std::set<QString>* dict);
    void LoadGaddag(const QString& path);
    void TestGaddag();
    std::set<WordString> GetAnagrams(const WordString& rack, bool must_use_all);
    void Anagram(const unsigned char* node, int* used_counts, int* counts,
                 uint32_t used_bits, uint32_t rack_bits, WordString* prefix,
                 std::vector<WordString>* anagrams, bool must_use_all);

    std::set<QString> twl;
    std::set<QString> csw;
    char* gaddag_bytes_;
    Gaddag* gaddag_;

    QLineEdit* answer_line_edit = nullptr;

    QLineEdit* rows_line_edit = nullptr;
    QLineEdit* cols_line_edit = nullptr;
    QLineEdit* num_words_line_edit = nullptr;

    QLineEdit* per_set_line_edit = nullptr;
    QLineEdit* per_word_line_edit = nullptr;
    QLineEdit* per_quiz_line_edit = nullptr;

    WordStatusBar* word_status_bar;

    size_t default_rows;
    size_t default_cols;
    size_t num_rows;
    size_t num_cols;
    size_t max_blank_words_per_rack;
    QString font_name;
    QFont::Weight font_weight;

    int timer_millis;
    bool choosing;
    bool paused;
    bool time_expired;
    bool quiz_finished;
    QElapsedTimer elapsed_timer;
    QBasicTimer timer;

    static Wordmonger* self;
};

#endif  // WORDMONGER_H
