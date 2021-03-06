#include <QFile>
#include <QGridLayout>
#include <QtGui>
#include <QtWidgets>

#include "gaddag.h"
#include "gaddag_maker.h"
#include "util.h"
#include "wordmonger.h"

Wordmonger* Wordmonger::self = 0;
Wordmonger* Wordmonger::get() { return self; }
Wordmonger::~Wordmonger() {}

Wordmonger::Wordmonger(QWidget* parent) : QMainWindow(parent) {
  self = this;

  default_rows = 9;
  default_cols = 5;

  num_rows = 9;
  num_cols = 5;
  max_blank_words_per_rack = 5;
  font_name = "Optima";
  font_weight = QFont::Black;

  CreateMenus();
  LoadDictionaries();
  CreateCentralWidgetAndLayout();
  CreateQuizChoiceWidgets();
  //CreateGridQuizWidgets();

  //ChooseWords();
  //DrawRacks();
  //AddQuestions();
  //StartTimer();
}

void Wordmonger::CreateMenus() {
  menu_bar = new QMenuBar(this);
  pause_action = new QAction(tr("&Pause"), this);
  pause_action->setShortcut(tr("Ctrl+P"));
  connect(pause_action, SIGNAL(triggered()), this, SLOT(TogglePauseSlot()));

  fullscreen_action = new QAction(tr("&Fullscreen"), this);
  fullscreen_action->setShortcut(tr("Ctrl+F"));
  connect(fullscreen_action, SIGNAL(triggered()), this,
          SLOT(ToggleFullscreenSlot()));
  quiz_menu = menu_bar->addMenu(tr("&Quiz"));
  quiz_menu->addAction(fullscreen_action);
  quiz_menu->addAction(pause_action);

}

namespace {
  QString Alphagram(const QString& word) {
    std::map<QChar, int> counts;
    for (const QChar& c : word) {
      counts[c]++;
    }
    QString vowels;
    QString consonants;
    for (const auto& pair : counts) {
      const QChar& c = pair.first;
      for (int i = 0; i < pair.second; ++i) {
        if (c == '?' || c == 'A' || c == 'E' || c == 'I' || c == 'O'
            || c == "U") {
          vowels += c;
        } else {
          consonants += c;
        }
      }
    }
    return vowels + consonants;
  }
}  // namespace

void Wordmonger::DrawRacks() {
  srand(time(nullptr));
  const Bag bag = Util::ScrabbleBag();
  vector<int> column_starts;
  std::set<QString> previous_answers;
  for (size_t col = 0; col < num_cols; ++col) {
    qInfo() << "col:" << col;
    column_starts.push_back(questions_and_answers.size());
    for (size_t row = 0; row < num_rows;) {
      qInfo() << "row: " << row;
      const size_t max_words_in_rack =
          std::min(num_rows - row, max_blank_words_per_rack);
      qInfo() << "max_words_in_rack:" << max_words_in_rack;
      const WordString rack = Util::BlankRack(bag, 1, 7);
      qInfo() << "rack:" << Util::DecodeWord(rack);
      std::set<WordString> words = GetAnagrams(rack, true);
      qInfo() << "words.size():" << words.size();
      if (words.size() < 1 || words.size() > max_words_in_rack) {
        continue;
      }
      const QString alpha = Alphagram(Util::DecodeWord(rack));
      std::vector<QString> answers;
      for (const WordString& word : words) {
        answers.push_back(Util::DecodeWord(word));
      }
      bool rack_has_repeat = false;
      for (const QString word : answers) {
        if (previous_answers.count(word) > 0) {
          rack_has_repeat = true;
        }
      }
      if (rack_has_repeat) continue;
      for (const QString word : answers) {
        previous_answers.insert(word);
      }
      QuestionAndAnswer q_and_a(alpha, answers);
      questions_and_answers.push_back(q_and_a);
      row += answers.size();
    }
  }
  column_starts.push_back(questions_and_answers.size());
  for (size_t i = 0; i < column_starts.size() - 1; i++) {
    std::random_shuffle(questions_and_answers.begin() + column_starts[i],
                        questions_and_answers.begin() + column_starts[i + 1]);
  }
}

void Wordmonger::CreateCentralWidgetAndLayout() {
  central_widget = new QWidget(this);
  central_layout = new QVBoxLayout(central_widget);
  central_layout->setContentsMargins(0, 0, 0, 0);
  central_layout->setSpacing(0);
  setCentralWidget(central_widget);

  setMinimumSize(640, 360);
  resize(1080, 602);
}

void Wordmonger::CreateQuizChoiceWidgets() {
  choosing = true;
  choosers_layout = new QGridLayout();
  central_layout->addLayout(choosers_layout);

  quiz_chooser = new QuizChooser(central_widget);
  choosers_layout->addWidget(quiz_chooser, 0, 0, 2, 1);
  quiz_chooser_layout = new QVBoxLayout(quiz_chooser);
  quiz_chooser_layout->setSpacing(0);

  word_length = new ChooserButtonRow(quiz_chooser, "Word Length", BUTTONS);
  word_length->AddButton("3", "length_3");
  word_length->AddButton("4", "length_4");
  word_length->AddButton("5", "length_5");
  word_length->AddButton("6", "length_6");
  word_length->AddButton("7", "length_7");
  word_length->AddButton("8", "length_8");
  word_length->AddButton("9", "length_9");
  quiz_chooser_layout->addWidget(word_length);

  word_builder = new ChooserButtonRow(quiz_chooser, "Word Builder", BUTTONS,
                                      ONE_IN_ROW);
  word_builder->AddButton("3-6", "builder_3_6");
  word_builder->AddButton("4-7", "builder_4_7");
  word_builder->AddButton("5-8", "builder_5_8");
  quiz_chooser_layout->addWidget(word_builder);

  blanks = new ChooserButtonRow(quiz_chooser, "With Blanks", BUTTONS);
  blanks->AddButton("6", "blanks_6");
  blanks->AddButton("7", "blanks_7");
  blanks->AddButton("8", "blanks_8");
  blanks->AddButton("9", "blanks_9");
  quiz_chooser_layout->addWidget(blanks);

  num_solutions = new ChooserButtonRow(quiz_chooser, "Number of Solutions",
                                       LINE_EDITS);
  num_solutions->AddLabelledLineEdit("MIN", 1, true);
  num_solutions->AddLabelledLineEdit("MAX", 1, true);
  num_solutions->AddLineEditsStretch();
  quiz_chooser_layout->addWidget(num_solutions);

  rows_and_columns = new ChooserButtonRow(quiz_chooser, "Grid Size",
                                          LINE_EDITS);
  rows_and_columns->AddLabelledLineEdit("ROWS", 5, true, &rows_line_edit);
  QObject::connect(rows_line_edit, SIGNAL(textChanged(QString)), this,
                   SLOT(RowsChangedSlot(QString)));
  rows_and_columns->AddLabel("×");  // MULTIPLICATION SIGN (U+00D7)
  rows_and_columns->AddLabelledLineEdit("COLS", 9, true, &cols_line_edit);
  QObject::connect(cols_line_edit, SIGNAL(textChanged(QString)), this,
                   SLOT(ColsChangedSlot(QString)));
  rows_and_columns->AddLabel("=");
  rows_and_columns->AddLabelledLineEdit("WORDS", 45, false,
                                        &num_words_line_edit, 42);
  rows_and_columns->AddLineEditsStretch();
  quiz_chooser_layout->addWidget(rows_and_columns);

  ordering = new ChooserButtonRow(quiz_chooser, "Ordering", BUTTONS);
  ordering->AddButton("RANDOM", "random");
  ordering->AddButton("PLAYABLE", "playability");
  ordering->AddButton("PROBABLE", "probability");

  quiz_chooser_layout->addWidget(ordering);

  quiz_time = new ChooserButtonRow(quiz_chooser, "Time", LINE_EDITS);
  quiz_time->AddLabelledLineEdit("SET (ms)", 2000, true, &per_set_line_edit,
                                 60, 0, 99999);
  quiz_time->AddLabelledLineEdit("WORD (ms)", 2000, true, &per_word_line_edit,
                                 60, 0, 99999);
  quiz_time->AddLabel("=");
  quiz_time->AddLabelledLineEdit("QUIZ (s)", 0, false, &per_quiz_line_edit,
                                 48, 0, 99999);
  quiz_time->AddLineEditsStretch();
  quiz_chooser_layout->addWidget(quiz_time);

  quiz_chooser_layout->setStretch(6, 1);

  quiz_preview = new Preview(central_widget);
  choosers_layout->addWidget(quiz_preview, 0, 1, 1, 1);

  detail_chooser = new DetailChooser(central_widget);
  choosers_layout->addWidget(detail_chooser, 1, 1, 1, 1);

  choosers_layout->setColumnStretch(0, 2);
  choosers_layout->setColumnStretch(1, 4);
  choosers_layout->setRowStretch(0, 4);
  choosers_layout->setRowStretch(1, 2);
}

int Wordmonger::RequestedRows() {
  if (rows_line_edit == nullptr) {
    return default_rows;
  }
  return rows_line_edit->text().toInt();
}

int Wordmonger::RequestedCols() {
  if (cols_line_edit == nullptr) {
    return default_cols;
  }
  return cols_line_edit->text().toInt();
}

void Wordmonger::CreateGridQuizWidgets() {
  choosing = false;
  questions_layout = new QGridLayout;
  central_layout->addLayout(questions_layout);

  answer_line_edit = new QLineEdit(central_widget);
  answer_line_edit->setAlignment(Qt::AlignHCenter);
  QObject::connect(answer_line_edit, SIGNAL(textChanged(QString)), this,
                   SLOT(TextChangedSlot(QString)));

  central_layout->addWidget(answer_line_edit);

  word_status_bar = new WordStatusBar(this);
  central_layout->addWidget(word_status_bar);

  central_layout->setStretchFactor(questions_layout, 3);
}

void Wordmonger::StartTimer() {
  timer_millis = 180 * 1000;
  elapsed_timer.start();
  timer.start(1000 / 60, this);
  time_expired = false;
  quiz_finished = false;
  paused = false;
}

void Wordmonger::PauseTimer() {
  paused = true;
  for (Question* q : questions) {
    q->SetBlurRadius(35);
    q->update();
  }
}

void Wordmonger::UnpauseTimer() {
  paused = false;
  for (Question* q : questions) {
    q->SetBlurRadius(0);
    q->update();
  }
}

void Wordmonger::LoadDictionaries() {
  LoadDictionary("/Users/john/scrabble/csw.txt", &csw);
  LoadDictionary("/Users/john/scrabble/twl.txt", &twl);

  /*
  GaddagMaker gaddag_maker(false, false);
  gaddag_maker.MakeGaddag("/Users/johnolaughlin/scrabble/csw15.txt",
                          "/Users/johnolaughlin/scrabble/csw15.gaddag");
  */
  LoadGaddag("/Users/johnolaughlin/scrabble/csw15.gaddag");
  //TestGaddag();
}

std::set<WordString> Wordmonger::GetAnagrams(const WordString& rack,
                                             bool must_use_all) {
  int counts[LAST_LETTER + 1];
  int used_counts[LAST_LETTER + 1];
  for (int i = BLANK; i <= LAST_LETTER; ++i) {
    counts[i] = 0;
    used_counts[i] = 0;
  }
  uint32_t rack_bits = 0;
  for (Letter letter : rack) {
    counts[letter]++;
    if (letter != BLANK) {
      rack_bits |= 1 << letter;
    }
  }
  //for (int i = BLANK; i <= LAST_LETTER; ++i) {
  //  qInfo() << "i:" << i << "counts[i]:" << counts[i];
  //}
  std::vector<WordString> anagrams;
  WordString prefix;

  int unused_bits = ~0;
  Anagram(gaddag_->Root(), used_counts, counts, unused_bits, rack_bits, &prefix,
          &anagrams, must_use_all);
  std::set<WordString> unique_words(anagrams.begin(), anagrams.end());
  //for (const WordString& word : unique_words) {
  // qInfo() << "word:" << Util::DecodeWord(word);
  //}
  //qInfo() << "found" << unique_words.size() << "unique words";
  return unique_words;
}

void Wordmonger::TestGaddag() {
  QString polish_blank = "POLISH??";
  WordString rack = Util::EncodeWord(polish_blank);
  std::set<WordString> unique_words = GetAnagrams(rack, true);
  for (const WordString& word : unique_words) {
    qInfo() << "word:" << Util::DecodeWord(word);
  }
  qInfo() << "found" << unique_words.size() << "unique words";
}

void Wordmonger::Anagram(const unsigned char* node, int* used_counts,
                         int* counts, uint32_t unused_bits,
                         uint32_t rack_bits, WordString* prefix,
                         std::vector<WordString>* anagrams,
                         bool must_use_all) {
//  qInfo() << "Anagram(...) prefix:" << Util::DecodeWord(*prefix)
//          << "unused_bits:" << Util::DecodeBits(unused_bits)
//          << "rack_bits:" << Util::DecodeBits(rack_bits)
//          << "used_counts:" << Util::DecodeCounts(used_counts)
//          << "counts: " << Util::DecodeCounts(counts);
  if (prefix->length() == 1) {
    node = gaddag_->FollowIndex(gaddag_->ChangeDirection(node));
  }
  if (counts[BLANK] > 0 || gaddag_->HasAnyChild(node, rack_bits)) {
    Letter min_letter = FIRST_LETTER;
    int child_index = 0;
    for (;;) {
      Letter found_letter;
      const unsigned char* child = nullptr;
      if (counts[BLANK] > 0) {
        child =
            gaddag_->NextRackChild(node, min_letter, unused_bits,
                                   &child_index, &found_letter);
        if (child == nullptr) {
          //qInfo() << "no child with letter past" << Util::DecodeLetter(min_letter);
          return;
        }
        assert(found_letter >= FIRST_LETTER);
        assert(found_letter <= LAST_LETTER);
        //qInfo() << "blank found_letter" << Util::DecodeLetter(found_letter);
        prefix->push_back(found_letter);
        counts[BLANK]--;
        if (gaddag_->CompletesWord(child)) {
          if (!must_use_all || (rack_bits == 0 && counts[BLANK] == 0)) {
            anagrams->push_back(*prefix);
          }
        }
        const unsigned char* new_node = gaddag_->FollowIndex(child);
        if (new_node != nullptr) {
          Anagram(new_node, used_counts, counts, unused_bits, rack_bits,
                  prefix, anagrams, must_use_all);
        } else {
          //qInfo() << "no new node for blank letter";
        }
        prefix->pop_back();
        counts[BLANK]++;
      } else {
        child = gaddag_->NextRackChild(node, min_letter, rack_bits,
                                       &child_index, &found_letter);
        if (child == nullptr) return;
      }
      if (counts[found_letter] > 0) {
        prefix->push_back(found_letter);
        counts[found_letter]--;
        used_counts[found_letter]++;
        const uint32_t found_letter_mask = 1 << found_letter;
        if (counts[found_letter] == 0) {
          rack_bits &= ~found_letter_mask;
          unused_bits &= ~found_letter_mask;
        }
        if (gaddag_->CompletesWord(child)) {
          if (!must_use_all || (rack_bits == 0 && counts[BLANK] == 0)) {
            anagrams->push_back(*prefix);
          }
        }
        const unsigned char* new_node = gaddag_->FollowIndex(child);
        if (new_node != nullptr) {
          Anagram(new_node, used_counts, counts, unused_bits, rack_bits,
                  prefix, anagrams, must_use_all);
        }
        prefix->pop_back();
        counts[found_letter]++;
        used_counts[found_letter]--;
        unused_bits |= found_letter_mask;
        rack_bits |= found_letter_mask;
      }
      min_letter = found_letter + 1;
      ++child_index;
    }
  }
}

void Wordmonger::LoadGaddag(const QString& path) {
  QFile input(path);
  char version_byte;
  if (input.open(QIODevice::ReadOnly)) {
    input.getChar(&version_byte);
    qInfo() << "version_byte:" << static_cast<int>(version_byte);
    char hash[16];
    input.read(hash, sizeof(hash));
    const int root_pos = input.pos();

    char last_letter;
    input.getChar(&last_letter);
    char bitset_size;
    input.getChar(&bitset_size);
    char index_size;
    input.getChar(&index_size);

    qInfo() << "root_pos:" << root_pos;
    const int size = input.size() - root_pos;
    qInfo() << "gaddag size:" << size;
    gaddag_bytes_ = new char[size];
    input.read(gaddag_bytes_, size);
    gaddag_ = new Gaddag(gaddag_bytes_, last_letter, bitset_size, index_size);
  }
  input.close();
}

void Wordmonger::timerEvent(QTimerEvent *event) {
  if (event->timerId() != timer.timerId()) {
    QWidget::timerEvent(event);
    return;
  }
  if (choosing || paused || time_expired || quiz_finished) {
    return;
  }
  const qint64 elapsed = std::min(50LL, elapsed_timer.restart());
  timer_millis -= elapsed;
  /*
  int radius = (180 * 1000 - timer_millis) / 100;
  qInfo() << "radius:" << radius;
  for (Question* q : questions) {
    q->SetBlurRadius(radius);
    q->update();
  }
  */
  if (timer_millis <= 0) {
      time_expired = true;
      quiz_finished = true;
      timer_millis = 0;
      for (Question* q : questions) {
        q->update();
      }
  }
  //qInfo() << "timer_millis: " << timer_millis;
  word_status_bar->SetTime(timer_millis);
  word_status_bar->update();
}

void Wordmonger::resizeEvent(QResizeEvent* event) {
  QLinearGradient linearGrad(0, 0, event->size().width(), event->size().height());
  int v_spacing = std::min(9, height() / 72);
  int h_spacing = std::min(9, width() / 120);
  const int spacing = std::min(v_spacing, h_spacing);

  if (choosing) {
    linearGrad.setColorAt(0, QColor(220, 230, 240));
    linearGrad.setColorAt(0.5, QColor(235, 240, 240));
    linearGrad.setColorAt(1, QColor(225, 225, 230));

    QPalette pal = palette();
    QBrush brush(linearGrad);
    pal.setBrush(QPalette::Window, brush);
    setAutoFillBackground(true);
    setPalette(pal);

    choosers_layout->setContentsMargins(spacing, spacing, spacing, spacing);
    choosers_layout->setHorizontalSpacing(spacing);
    choosers_layout->setVerticalSpacing(spacing);
    return;
  }

  linearGrad.setColorAt(0, QColor(202, 210, 220));
  linearGrad.setColorAt(0.5, QColor(210, 221, 225));
  linearGrad.setColorAt(1, QColor(201, 207, 216));

  QPalette pal = palette();
  QBrush brush(linearGrad);
  pal.setBrush(QPalette::Window, brush);
  setAutoFillBackground(true);
  setPalette(pal);

  questions_layout->setContentsMargins(spacing, spacing, spacing, spacing);
  questions_layout->setHorizontalSpacing(spacing);
  questions_layout->setVerticalSpacing(spacing);

  qInfo() << "questions_layout->horizonalSpacing(): "
          << questions_layout->horizontalSpacing();

  const int line_edit_font_size = std::min(18, std::max(12, height() / 35));
  QFont font(Wordmonger::get()->FontName(), line_edit_font_size,
             Wordmonger::get()->FontWeight());
  qInfo() << "line_edit_font_size: " << line_edit_font_size;
  answer_line_edit->setFont(font);

  const int status_bar_font_size = std::min(16, std::max(10, height() / 42));
  qInfo() << "status_bar_font_size: " << status_bar_font_size;
  word_status_bar->SetFontSize(status_bar_font_size);
  word_status_bar->setMinimumHeight(line_edit_font_size * 1.75);
}

QString QuestionAndAnswer::GetClue() const {
  return Alphagram(clue);
}

void Wordmonger::AddQuestions() {
 int i = 0;
 for (size_t col = 0; col < num_cols; col++) {
   for (size_t row = 0; row < num_rows;) {
     const std::vector<QString>& answers =
         questions_and_answers[i].GetAnswers();
     if (answers.size() + row > num_rows) {
       break;
     }
     Question* question = new Question(this, i);
     questions.push_back(question);
     questions_layout->addWidget(question, row, col, answers.size(), 1);
     for (const QString& answer : answers) {
       //qInfo() << "answer: " << answer << " i: " << i;
       answer_map[answer].push_back(question);
     }
     row += answers.size();
     i++;
   }
  }
}

void Wordmonger::CheckIfQuizFinished() {
  for (Question* question : questions) {
    if (!question->IsAllSolved()) {
      return;
    }
  }
  quiz_finished = true;
  word_status_bar->update();
}

QuizChooser::QuizChooser(QWidget *parent) {}

void QuizChooser::resizeEvent(QResizeEvent *event) {
  int v_spacing = std::max(4, height() / 100);
  int h_spacing = std::max(4, width() / 80);
  const int spacing = std::min(v_spacing, h_spacing);

  QLinearGradient linearGrad(0, 0, event->size().width(),
                             event->size().height());
  linearGrad.setColorAt(0, QColor(202, 210, 220, 128));
  linearGrad.setColorAt(0.5, QColor(210, 221, 225, 128));
  linearGrad.setColorAt(1, QColor(201, 207, 216, 128));

  QPalette pal = palette();
  QBrush brush(linearGrad);
  pal.setBrush(QPalette::Window, brush);
  setAutoFillBackground(true);
  setPalette(pal);

  Wordmonger::get()->QuizChooserLayout()->setContentsMargins(
        spacing, spacing, spacing, spacing);
  Wordmonger::get()->QuizChooserLayout()->setSpacing(spacing);
}

ChooserButtonRow::ChooserButtonRow(QWidget *parent,
                                   const QString& label_text,
                                   ChooserType chooser_type,
                                   enum ButtonType button_type) {
  button_type_ = button_type;
  layout = new QVBoxLayout;
  layout->setContentsMargins(4, 4, 4, 4);
  layout->setSpacing(0);
  setLayout(layout);
  label = new QLabel(this);
  label->setText(label_text);
  label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  layout->addWidget(label, 0, Qt::AlignTop);

  switch (chooser_type) {
    case BUTTONS:
      buttons = new QWidget(this);
      buttons->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
      buttons_layout = new QHBoxLayout;
      buttons_layout->setSpacing(2);
      buttons_layout->setContentsMargins(4, 4, 4, 4);
      buttons->setLayout(buttons_layout);
      layout->addWidget(buttons, 1, Qt::AlignTop);
      break;
   case LINE_EDITS:
      line_edits = new QWidget(this);
      line_edits->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
      line_edits_layout = new QHBoxLayout;
      line_edits_layout->setSpacing(8);
      line_edits_layout->setContentsMargins(4, 4, 4, 4);
      line_edits->setLayout(line_edits_layout);
      layout->addWidget(line_edits, 1, Qt::AlignTop);
      break;
  }
}

void ChooserButtonRow::AddButton(const QString& label, const QString& id) {
  buttons_list << new QuizPushButton(this, buttons);
  buttons_list.back()->setText(label);
  buttons_list.back()->setCheckable(true);
  buttons_list.back()->setStyleSheet(R"(
    QPushButton {
      background-color: rgba(255, 255, 255, 50%);
      color: rgba(0, 0, 0, 80%);
      border-style: solid;
      border-color: #404040;
      border-width: 1px;
    }
    QPushButton:hover {
      background-color: rgba(240, 240, 240, 50%);
    }
    QPushButton:pressed {
      background-color: rgba(68, 125, 140, 30%);
    }
    QPushButton:checked {
      background-color: rgba(88, 125, 140, 40%);
      color: rgba(20, 20, 20, 100%);
    }
  )");
  QFont font(Wordmonger::get()->FontName(), 20,
             Wordmonger::get()->FontWeight());
  buttons_list.back()->setFont(font);
  connect(buttons_list.back(), SIGNAL(released()), Wordmonger::get(),
                                      SLOT(ButtonReleasedSlot()));
  buttons_layout->addWidget(buttons_list.back(), 0, Qt::AlignTop);
}

void ChooserButtonRow::AddLabelledLineEdit(const QString& label,
                                           int value,
                                           bool editable,
                                           QLineEdit** line_edit_ptr,
                                           int width, int min, int max) {
  LabelledLineEdit* lle =
      new LabelledLineEdit(line_edits, label, value, editable,
                           line_edit_ptr, width, min, max);
  line_edits_list << lle;
  line_edits_layout->addWidget(line_edits_list.back());
}

void ChooserButtonRow::AddLabel(const QString& label_text) {
  QLabel* label = new QLabel;
  label->setText(label_text);
  QFont font(Wordmonger::get()->FontName(), 20,
             Wordmonger::get()->FontWeight());
  label->setFont(font);
  label->setContentsMargins(0, 16, 0, 0);
  line_edits_list << label;
  line_edits_layout->addWidget(line_edits_list.back());
}

void ChooserButtonRow::AddLineEditsStretch() {
  line_edits_layout->addStretch(1);
}

QuizPushButton::QuizPushButton(ChooserButtonRow *parent_row, QWidget *parent) {
  parent_row_ = parent_row;
}

void ChooserButtonRow::resizeEvent(QResizeEvent *event) {
  QLinearGradient linearGrad(0, 0, event->size().width(),
                             event->size().height());
  linearGrad.setColorAt(0, QColor(250, 250, 250, 128));
  linearGrad.setColorAt(0.5, QColor(255, 255, 225, 128));
  linearGrad.setColorAt(1, QColor(250, 250, 250, 128));

  QPalette pal = palette();
  QBrush brush(linearGrad);
  pal.setBrush(QPalette::Window, brush);
  setAutoFillBackground(true);
  setPalette(pal);

  if (buttons_list.size() > 0) {
    const int button_height =
        std::min(0.9 * event->size().width() / buttons_list.size(),
                 0.6 * event->size().height());
    for (QWidget* button : buttons_list) {
      button->setMinimumHeight(button_height);
    }
  }
}

LabelledLineEdit::LabelledLineEdit(QWidget *parent, const QString& label_text,
                                   int value, bool editable,
                                   QLineEdit** line_edit_ptr, int width,
                                   int min, int max) {
  layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(4);
  setLayout(layout);

  label = new QLabel(this);
  label->setText(label_text);
  QFont font(Wordmonger::get()->FontName(), 12,
             Wordmonger::get()->FontWeight());
  label->setFont(font);
  label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  layout->addWidget(label, 0, Qt::AlignTop);

  line_edit = new QLineEdit(this);
  QFont line_edit_font(Wordmonger::get()->FontName(), 18,
                       Wordmonger::get()->FontWeight());
  line_edit->setFont(line_edit_font);
  line_edit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
  line_edit->setFixedWidth(width);
  if (editable) {
    line_edit->setValidator(new QIntValidator(min, max, this));
  } else {
    line_edit->setDisabled(true);
  }
  line_edit->setText(QString::number(value));
  if (line_edit_ptr != nullptr) {
    *line_edit_ptr = line_edit;
  }
  layout->addWidget(line_edit, 0, Qt::AlignTop);
}

Preview::Preview(QWidget *parent) {}

void Preview::resizeEvent(QResizeEvent *event) {
  QLinearGradient linearGrad(0, 0, event->size().width(),
                             event->size().height());
  linearGrad.setColorAt(0, QColor(202, 210, 220));
  linearGrad.setColorAt(0.5, QColor(210, 221, 225));
  linearGrad.setColorAt(1, QColor(201, 207, 216));

  QPalette pal = palette();
  QBrush brush(linearGrad);
  pal.setBrush(QPalette::Window, brush);
  setAutoFillBackground(true);
  setPalette(pal);
}

DetailChooser::DetailChooser(QWidget *parent) {}

void DetailChooser::resizeEvent(QResizeEvent *event) {
  QLinearGradient linearGrad(0, 0, event->size().width(),
                             event->size().height());
  linearGrad.setColorAt(0, QColor(202, 210, 220, 128));
  linearGrad.setColorAt(0.5, QColor(210, 221, 225, 128));
  linearGrad.setColorAt(1, QColor(201, 207, 216, 128));

  QPalette pal = palette();
  QBrush brush(linearGrad);
  pal.setBrush(QPalette::Window, brush);
  setAutoFillBackground(true);
  setPalette(pal);
}

WordStatusBar::WordStatusBar(QWidget *parent) {}

void WordStatusBar::paintEvent(QPaintEvent* event) {
  QPainter painter;
  painter.begin(this);
  painter.eraseRect(event->rect());

  double time_seconds = time_millis / 1000.0;
  const int decimal_places =
    (Wordmonger::get()->QuizFinished() || time_seconds < 10) ? 1 : 0;
  QString time_string = QString::number(time_seconds, 'f', decimal_places);

  QFont font(Wordmonger::get()->FontName(), font_size,
             Wordmonger::get()->FontWeight());
  QFontMetrics fm(font);
  int text_width = fm.width(time_string);
  int text_height = fm.height();
  int descent = fm.descent();
  painter.setFont(font);

  int x = 0.5 * (width() - text_width);
  int y = 0.5 * (height() + text_height) - descent;
  painter.drawText(x, y, time_string);
}

Question::Question(QWidget* parent, int index) {
  this->index = index;
  blur_effect = new QGraphicsBlurEffect(this);
  blur_effect->setBlurRadius(0);
  setGraphicsEffect(blur_effect);
}

bool Question::IsAllSolved() const {
  const QuestionAndAnswer& q_and_a =
    Wordmonger::get()->QuestionAndAnswerAt(index);
  return solved_answer_indices.size() == q_and_a.GetAnswers().size();
}

void Question::MarkAnswer(const QString& given_answer) {
  qInfo() << "MarkAnswer(" << given_answer << ")...";
  const QuestionAndAnswer& q_and_a =
      Wordmonger::get()->QuestionAndAnswerAt(index);
  int answer_index = 0;
  for (const QString answer : q_and_a.GetAnswers()) {
    qInfo() << "answer: " << answer;
    if (answer == given_answer) {
      qInfo() << "solved answer #" << answer_index;
      solved_answer_indices.insert(answer_index);
    }
    answer_index++;
    qInfo() << "new answer_index: " << answer_index;
  }
  Wordmonger::get()->CheckIfQuizFinished();
  update();
}

void Question::paintEvent(QPaintEvent* event) {
  const QuestionAndAnswer& q_and_a =
      Wordmonger::get()->QuestionAndAnswerAt(index);
  const QString clue = q_and_a.GetClue();
  const bool quiz_finished = Wordmonger::get()->QuizFinished();
  QPainter painter;
  painter.begin(this);
  painter.eraseRect(event->rect());
  int clue_height = height();
  const auto& answers = q_and_a.GetAnswers();
  int num_subcells = 1;
  const bool partially_revealed =
      !quiz_finished && (solved_answer_indices.size() > 0) &&
      (solved_answer_indices.size() < answers.size());
  const bool wrong = quiz_finished &&
      (solved_answer_indices.size() < answers.size());
  //qInfo() << "partially_revealed: " << partially_revealed;
  if (solved_answer_indices.size() == 0 && !quiz_finished) {
    num_subcells = 1;
  } else if (partially_revealed) {
    num_subcells = answers.size() + 1;
  } else {
    num_subcells = answers.size();
  }
  //qInfo() << "num_subcells: " << num_subcells;
  std::vector<int> fill_dividing_line_heights;
  std::vector<int> border_dividing_line_heights;
  for (int i = 0; i <= num_subcells; i++) {
    int y = (i * height() - 1) / num_subcells;
    fill_dividing_line_heights.push_back(y);
    if (i == num_subcells) {
      y--;
    }
    border_dividing_line_heights.push_back(y);
  }
  clue_height = fill_dividing_line_heights[1] - fill_dividing_line_heights[0];
  int clue_font_size = 0.5 * clue_height;
  int answer_font_size = 0.45 * clue_height;
  if (!quiz_finished && (solved_answer_indices.size() == 0 ||
                         solved_answer_indices.size() < answers.size())) {
    QRect rect(0, 0, width(), clue_height + 1);
    painter.fillRect(rect, QColor(255, 255, 255, 80));

    if (solved_answer_indices.size() > 0) {
      QRect border_rect(0, 0, width() - 1, clue_height);
      painter.setPen({0, 0, 0, 32});
      painter.drawRect(border_rect);
    }

    QFont font(Wordmonger::get()->FontName(), clue_font_size,
               Wordmonger::get()->FontWeight());
    QFontMetrics fm(font);
    int text_width = fm.width(clue);
    int text_height = fm.height();
    int descent = fm.descent();
    const int max_text_width = 0.75 * width();
    if (text_width > max_text_width) {
      clue_font_size = max_text_width * clue_font_size / text_width;
      clue_font_size = std::max(6, clue_font_size);
      QFont smaller_font(Wordmonger::get()->FontName(), clue_font_size,
                         Wordmonger::get()->FontWeight());
      QFontMetrics fm(smaller_font);
      text_width = fm.width(clue);
      text_height = fm.height();
      descent = fm.descent();
    }
    painter.setFont({Wordmonger::get()->FontName(), clue_font_size,
                     Wordmonger::get()->FontWeight()});

    int x = 0.5 * (width() - text_width);
    int y = 0.5 * (clue_height + text_height) - descent;
    painter.drawText(x, y, clue);
  }
  if (!solved_answer_indices.empty() || quiz_finished) {
    for (unsigned int i = 0; i < answers.size(); ++i) {
      const int top_dividing_index =
          partially_revealed ? i + 1 : i;
      const int bottom_dividing_index = top_dividing_index + 1;
      const int top = fill_dividing_line_heights[top_dividing_index];
      const int bottom = fill_dividing_line_heights[bottom_dividing_index];
      const int border_bottom =
          border_dividing_line_heights[bottom_dividing_index];
      //qInfo() << "i: " << i << " bottom: " << bottom << " top: " << top;
      const int answer_height = bottom - top;
      const int answer_border_height = border_bottom - top;
      QRect rect(0, top + 1, width() - 1, answer_height);
      QRect border_rect(0, top + 1, width() - 1, answer_border_height);
      //qInfo() << "i: " << i << ", rect({" << rect.left() << ", " << rect.bottom()
      //        << ", " << rect.right() << ", " << rect.top() << "})";
      if (solved_answer_indices.count(i) == 0) {
        painter.fillRect(rect, QColor(255, 255, 255, 48));
      } else {
        painter.fillRect(rect, QColor(0, 0, 0, 20));
      }
      painter.setPen({0, 0, 0, 96});
      painter.drawRect(border_rect);

      const QString& answer = q_and_a.GetAnswers()[i];
      QString display_answer = answer;
      const bool solved_this_answer = solved_answer_indices.count(i) > 0;
      if (!quiz_finished && !solved_this_answer) {
        display_answer = "";
        for (int j = 0 ; j < answer.length();  j++) {
            display_answer += "–";
        }
      }
      //qInfo() << "painting text for answer: " << answer;
      QFont font(Wordmonger::get()->FontName(), answer_font_size,
                 Wordmonger::get()->FontWeight());
      QFontMetrics fm(font);
      int text_width = fm.width(display_answer);
      int text_height = fm.height();
      int descent = fm.descent();
      const int max_text_width = 0.75 * width();
      if (text_width > max_text_width) {
        answer_font_size = max_text_width * answer_font_size / text_width;
        answer_font_size = std::max(6, answer_font_size);
        QFont smaller_font(Wordmonger::get()->FontName(), answer_font_size,
                           Wordmonger::get()->FontWeight());
        QFontMetrics fm(smaller_font);
        text_width = fm.width(answer);
        text_height = fm.height();
        descent = fm.descent();
      }

      painter.setFont({Wordmonger::get()->FontName(), answer_font_size,
                       Wordmonger::get()->FontWeight()});
      if (Wordmonger::get()->IsTwl(answer)) {
        painter.setPen({0, 0, 0, solved_this_answer ? 224 : 255});
      } else {
        painter.setPen({200, 0, 0, solved_this_answer ? 224 : 255});
      }

      int x = 0.5 * (width() - text_width);
      int y = top + 0.5 * (answer_height + text_height) - descent;
      painter.drawText(x, y, display_answer);
    }
  }
  if (wrong) {
    QPen pen({0, 0, 20, 196});
    painter.setPen(pen);
    for (int offset = 0; offset < 3; offset++) {
      QRect wrong_border(offset, offset,
                         width() - 1 - 2 * offset, height() - 1 - 2 * offset);
      painter.drawRect(wrong_border);
    }
  }
  painter.end();
}

void Wordmonger::ChooseWords() {
  std::map<QString, std::vector<QString>> sets;
  for (const QString& word : csw) {
    if (word.length() != 7) {
      continue;
    }
    const QString alpha = Alphagram(word);
    sets[alpha].push_back(word);
  }

  std::vector<std::pair<QString, std::vector<QString>>> pairs(
        sets.begin(), sets.end());
  std::random_shuffle(pairs.begin(), pairs.end());
  size_t i = 0;
  for (const auto& pair : pairs) {
    if (pair.second.size() != 1) {
      continue;
    }
    if (i >= num_cols * num_rows) {
      return;
    }
    i++;
    QuestionAndAnswer q_and_a(pair.first, pair.second);
    questions_and_answers.push_back(q_and_a);
  }
}

void Wordmonger::LoadSingleAnagramWords() {
  QFile input("/Users/johnolaughlin/scrabble/csw-single-7s.txt");
  int i = 0;
  if (input.open(QIODevice::ReadOnly)) {
    QTextStream in(&input);
    while (!in.atEnd() && i < 45) {
      QString word = in.readLine();
      //qInfo() << "word: " << word;
      QuestionAndAnswer q_and_a(word, {word});
      questions_and_answers.push_back(q_and_a);
      ++i;
    }
    input.close();
  }
  qInfo() << "#words: " << i;
}

void Wordmonger::LoadDictionary(const QString& path, std::set<QString>* dict) {
  QFile input(path);
  if (input.open(QIODevice::ReadOnly)) {
    QTextStream in(&input);
    while (!in.atEnd()) {
      QString word = in.readLine();
      dict->insert(word);
    }
    input.close();
  }
  qInfo() << "loaded " << dict->size() << " from " << path;
}

QuestionAndAnswer::QuestionAndAnswer(const QString& clue,
                                     const std::vector<QString>& answers) {
  this->clue = clue;
  this->answers = answers;
}
