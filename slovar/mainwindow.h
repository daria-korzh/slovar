#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>

#include <vector>
#include <string>

using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

private:

    Ui::MainWindow *ui;


    vector<string> dictionary;


    // всплывающее окно подсказок
    QWidget *suggestionWidget;
    QHBoxLayout *suggestionLayout;


    void loadDictionary();


    bool binarySearch(const string& word);


    int levenshteinDistance(
        const string& s1,
        const string& s2);


    vector<pair<string,int>> findSimilarWords(
        const string& word,
        int maxDistance = 2);


    vector<string> findWordsByPrefix(
        const string& prefix);


    void showSuggestions(
        vector<pair<string,int>> words);

    vector<string> getWordsByPrefix(string prefix);



private slots:

    void checkWord();

    void clearFields();


    void onWordInputChanged(
        const QString& text);




public:

    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

};

#endif // MAINWINDOW_H