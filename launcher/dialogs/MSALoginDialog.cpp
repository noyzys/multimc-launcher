/* Copyright 2013-2021 MultiMC Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "MSALoginDialog.h"
#include "ui_MSALoginDialog.h"

#include "minecraft/auth/AccountTask.h"

#include <QtWidgets/QPushButton>

MSALoginDialog::MSALoginDialog(QWidget *parent) : QDialog(parent), ui(new Ui::MSALoginDialog)
{
    ui->setupUi(this);
    ui->progressBar->setVisible(false);
    // ui->buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(false);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

int MSALoginDialog::exec() {
    setUserInputsEnabled(false);
    ui->progressBar->setVisible(true);

    // Setup the login task and start it
    m_account = MinecraftAccount::createBlankMSA();
    m_loginTask = m_account->loginMSA(nullptr);
    connect(m_loginTask.get(), &Task::failed, this, &MSALoginDialog::onTaskFailed);
    connect(m_loginTask.get(), &Task::succeeded, this, &MSALoginDialog::onTaskSucceeded);
    connect(m_loginTask.get(), &Task::status, this, &MSALoginDialog::onTaskStatus);
    connect(m_loginTask.get(), &Task::progress, this, &MSALoginDialog::onTaskProgress);
    m_loginTask->start();

    return QDialog::exec();
}


MSALoginDialog::~MSALoginDialog()
{
    delete ui;
}

void MSALoginDialog::setUserInputsEnabled(bool enable)
{
    ui->buttonBox->setEnabled(enable);
}

void MSALoginDialog::onTaskFailed(const QString &reason)
{
    // Set message
    auto lines = reason.split('\n');
    QString processed;
    for(auto line: lines) {
        if(line.size()) {
            processed += "<font color='red'>" + line + "</font>\n";
        }
        else {
            processed += '\n';
        }
    }
    ui->label->setText(processed);

    // Re-enable user-interaction
    setUserInputsEnabled(true);
    ui->progressBar->setVisible(false);
}

void MSALoginDialog::onTaskSucceeded()
{
    QDialog::accept();
}

void MSALoginDialog::onTaskStatus(const QString &status)
{
    ui->label->setText(status);
}

void MSALoginDialog::onTaskProgress(qint64 current, qint64 total)
{
    ui->progressBar->setMaximum(total);
    ui->progressBar->setValue(current);
}

// Public interface
MinecraftAccountPtr MSALoginDialog::newAccount(QWidget *parent, QString msg)
{
    MSALoginDialog dlg(parent);
    dlg.ui->label->setText(msg);
    if (dlg.exec() == QDialog::Accepted)
    {
        return dlg.m_account;
    }
    return 0;
}