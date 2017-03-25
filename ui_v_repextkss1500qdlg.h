/********************************************************************************
** Form generated from reading UI file 'v_repextkss1500qdlg.ui'
**
** Created by: Qt User Interface Compiler version 5.6.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_V_REPEXTKSS1500QDLG_H
#define UI_V_REPEXTKSS1500QDLG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QTableWidget>

QT_BEGIN_NAMESPACE

class Ui_CKss1500RobotQDlg
{
public:
    QGroupBox *groupBox;
    QListWidget *listWidget;
    QGroupBox *groupBox_2;
    QTableWidget *tableWidget;

    void setupUi(QDialog *CKss1500RobotQDlg)
    {
        if (CKss1500RobotQDlg->objectName().isEmpty())
            CKss1500RobotQDlg->setObjectName(QStringLiteral("CKss1500RobotQDlg"));
        CKss1500RobotQDlg->resize(400, 313);
        groupBox = new QGroupBox(CKss1500RobotQDlg);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setGeometry(QRect(20, 12, 361, 138));
        listWidget = new QListWidget(groupBox);
        listWidget->setObjectName(QStringLiteral("listWidget"));
        listWidget->setGeometry(QRect(10, 20, 341, 111));
        groupBox_2 = new QGroupBox(CKss1500RobotQDlg);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        groupBox_2->setGeometry(QRect(20, 150, 361, 151));
        tableWidget = new QTableWidget(groupBox_2);
        if (tableWidget->columnCount() < 5)
            tableWidget->setColumnCount(5);
        if (tableWidget->rowCount() < 20)
            tableWidget->setRowCount(20);
        tableWidget->setObjectName(QStringLiteral("tableWidget"));
        tableWidget->setGeometry(QRect(10, 21, 341, 120));
        tableWidget->setAutoFillBackground(false);
        tableWidget->setRowCount(20);
        tableWidget->setColumnCount(5);

        retranslateUi(CKss1500RobotQDlg);

        QMetaObject::connectSlotsByName(CKss1500RobotQDlg);
    } // setupUi

    void retranslateUi(QDialog *CKss1500RobotQDlg)
    {
        CKss1500RobotQDlg->setWindowTitle(QApplication::translate("CKss1500RobotQDlg", "Dialog", 0));
        groupBox->setTitle(QApplication::translate("CKss1500RobotQDlg", "\352\260\200\354\203\201 KSS \354\212\244\354\271\264\353\235\274\353\241\234\353\264\207 \353\252\251\353\241\235 \353\260\217 \355\206\265\354\213\240 \354\227\260\352\262\260 \354\204\240\355\203\235", 0));
        groupBox_2->setTitle(QApplication::translate("CKss1500RobotQDlg", "\354\204\240\355\203\235\353\220\234 \352\260\235\354\262\264\354\235\230 \354\202\254\354\232\251\354\236\220 \355\217\254\354\235\270\355\212\270 \354\240\225\353\263\264", 0));
    } // retranslateUi

};

namespace Ui {
    class CKss1500RobotQDlg: public Ui_CKss1500RobotQDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_V_REPEXTKSS1500QDLG_H
