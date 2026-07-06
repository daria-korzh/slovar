#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <algorithm>
#include <cctype>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    setWindowTitle("Проверка правописания русских слов");
    setMinimumSize(600,500);


    loadDictionary();



    // создание горизонтального окна подсказок

    suggestionWidget = new QWidget(this);

    suggestionLayout = new QHBoxLayout(suggestionWidget);

    suggestionWidget->setLayout(suggestionLayout);



    suggestionWidget->setStyleSheet(
        "QWidget {"
        "background:white;"
        "border:1px solid gray;"
        "border-radius:10px;"
        "}"

        "QPushButton {"
        "border:none;"
        "padding:8px 15px;"
        "font-size:14px;"
        "}"

        "QPushButton:hover {"
        "background:#dddddd;"
        "}"
        );


    suggestionWidget->hide();



    connect(ui->checkButton,
            &QPushButton::clicked,
            this,
            &MainWindow::checkWord);



    connect(ui->clearButton,
            &QPushButton::clicked,
            this,
            &MainWindow::clearFields);



    connect(ui->wordInput,
            &QLineEdit::returnPressed,
            this,
            &MainWindow::checkWord);



    connect(ui->wordInput,
            &QLineEdit::textChanged,
            this,
            &MainWindow::onWordInputChanged);

}



MainWindow::~MainWindow()
{
    delete ui;
}



// ===========================
// Загрузка словаря
// ===========================


void MainWindow::loadDictionary()
{

    QString filePath = "russian.txt";


    QFile file(filePath);


    if(!file.exists())
    {
        QMessageBox::critical(
            this,
            "Ошибка",
            "Файл russian.txt не найден!");

        return;
    }



    if(!file.open(QIODevice::ReadOnly |
                   QIODevice::Text))
    {
        QMessageBox::critical(
            this,
            "Ошибка",
            "Не удалось открыть словарь!");

        return;
    }



    dictionary.clear();



    QByteArray data =
        file.readAll();


    file.close();



    QString content =
        QString::fromUtf8(data);



    QStringList lines =
        content.split('\n',
                      Qt::SkipEmptyParts);



    for(QString line : lines)
    {

        QString word =
            line.trimmed();



        if(!word.isEmpty())
        {

            string w =
                word.toStdString();



            transform(w.begin(),
                      w.end(),
                      w.begin(),
                      ::tolower);



            dictionary.push_back(w);
        }

    }



    sort(dictionary.begin(),
         dictionary.end());



    dictionary.erase(
        unique(dictionary.begin(),
               dictionary.end()),
        dictionary.end());



    ui->wordCountLabel->setText(
        QString("Слов в словаре: %1")
            .arg(dictionary.size()));

}



// ===========================
// Бинарный поиск
// ===========================


bool MainWindow::binarySearch(
    const string& word)
{

    int left = 0;

    int right =
        dictionary.size()-1;



    while(left <= right)
    {

        int mid =
            left+(right-left)/2;



        if(dictionary[mid]==word)
            return true;



        if(dictionary[mid]<word)

        left = mid+1;

        else

        right = mid-1;

    }


    return false;

}



// ===========================
// Расстояние Левенштейна
// ===========================


int MainWindow::levenshteinDistance(
    const string& s1,
    const string& s2)
{

    int m=s1.size();

    int n=s2.size();



    vector<vector<int>> dp(
        m+1,
        vector<int>(n+1));



    for(int i=0;i<=m;i++)
        dp[i][0]=i;


    for(int j=0;j<=n;j++)
        dp[0][j]=j;



    for(int i=1;i<=m;i++)
    {

        for(int j=1;j<=n;j++)
        {

            if(s1[i-1]==s2[j-1])

            dp[i][j]=dp[i-1][j-1];


            else

            dp[i][j]=min(
                {
                    dp[i-1][j]+1,
                    dp[i][j-1]+1,
                    dp[i-1][j-1]+1
                });

        }

    }


    return dp[m][n];

}
// ===========================
// Поиск похожих слов
// ===========================

vector<pair<string,int>> MainWindow::findSimilarWords(
    const string& word,
    int maxDistance)
{

    vector<pair<string,int>> similar;


    for(const string& dictWord : dictionary)
    {

        int distance =
            levenshteinDistance(
                word,
                dictWord);


        if(distance <= maxDistance &&
            dictWord != word)
        {
            similar.push_back(
                make_pair(dictWord,distance));
        }

    }



    sort(similar.begin(),
         similar.end(),
         [](const pair<string,int>& a,
            const pair<string,int>& b)
         {
             return a.second < b.second;
         });



    if(similar.size() > 3)
        similar.resize(3);



    return similar;
}



// ===========================
// Горизонтальное окно подсказок
// ===========================

void MainWindow::showSuggestions(
    vector<pair<string,int>> words)
{

    while(suggestionLayout->count())
    {
        QLayoutItem *item =
            suggestionLayout->takeAt(0);

        delete item->widget();
        delete item;
    }



    for(auto word : words)
    {

        QPushButton *button =
            new QPushButton(
                QString::fromStdString(word.first));


        connect(button,
                &QPushButton::clicked,
                this,
                [=]()
                {
                    ui->wordInput->setText(
                        QString::fromStdString(word.first));

                    suggestionWidget->hide();
                });



        suggestionLayout->addWidget(button);

    }



    suggestionWidget->adjustSize();



    // правильное расположение под строкой ввода

    QPoint globalPos =
        ui->wordInput->mapToGlobal(
            QPoint(0,
                   ui->wordInput->height()));



    QPoint localPos =
        this->mapFromGlobal(globalPos);



    suggestionWidget->move(localPos);



    suggestionWidget->show();

    suggestionWidget->raise();

}
vector<string> MainWindow::getWordsByPrefix(string prefix)
{
    vector<string> result;


    for(string word : dictionary)
    {

        if(word.size() >= prefix.size())
        {

            bool ok = true;


            for(int i=0;i<prefix.size();i++)
            {
                if(word[i] != prefix[i])
                {
                    ok=false;
                    break;
                }
            }



            if(ok)
            {
                result.push_back(word);


                if(result.size()==3)
                    break;
            }
        }
    }


    return result;
}
// ===========================
// Обработка изменения слова
void MainWindow::onWordInputChanged(
    const QString& text)
{
    string word = text.toStdString();


    transform(word.begin(),
              word.end(),
              word.begin(),
              ::tolower);



    // меньше двух букв — подсказки не показываем
    if(word.length() < 2)
    {
        suggestionWidget->hide();
        return;
    }



    vector<string> words =
        getWordsByPrefix(word);



    vector<pair<string,int>> result;



    for(string w : words)
    {
        result.push_back(
            {w,0});
    }



    if(!result.empty())
    {
        showSuggestions(result);
    }
    else
    {
        suggestionWidget->hide();
    }
}
// Проверка слова
// ===========================

void MainWindow::checkWord()
{

    if(dictionary.empty())
    {
        QMessageBox::warning(
            this,
            "Ошибка",
            "Словарь пуст!");

        return;
    }



    string word =
        ui->wordInput->text()
            .toStdString();



    transform(word.begin(),
              word.end(),
              word.begin(),
              ::tolower);



    if(word.empty())
    {
        QMessageBox::warning(
            this,
            "Ошибка",
            "Введите слово!");

        return;
    }



    QString result;



    result +=
        "<b>Результат проверки:</b><br><br>";



    bool found =
        binarySearch(word);



    if(found)
    {

        result +=
            "✅ Слово найдено в словаре!<br>";

        result +=
            "Написание правильное.";

    }

    else
    {

        result +=
            "❌ Слово не найдено в словаре.<br><br>";



        vector<pair<string,int>> similar =
            findSimilarWords(word,2);



        if(!similar.empty())
        {

            result +=
                "<b>Возможно, вы имели в виду:</b><br>";



            for(auto item : similar)
            {

                result +=
                    "• "
                    + QString::fromStdString(item.first)
                    + " (ошибка: "
                    + QString::number(item.second)
                    + ")<br>";

            }

        }

        else
        {

            result +=
                "Похожих слов не найдено.";

        }

    }



    ui->resultDisplay->setHtml(result);


    ui->statusLabel->setText(
        "Готово");

}



// ===========================
// Очистка
// ===========================

void MainWindow::clearFields()
{

    ui->wordInput->clear();

    ui->resultDisplay->clear();


    suggestionWidget->hide();


    ui->statusLabel->setText(
        "Очищено");


    ui->wordInput->setFocus();

}