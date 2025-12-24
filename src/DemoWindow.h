#pragma once

#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

#include "CalibrationModel.h"

// 简易演示窗口：
// 1. 使用模拟数据完成标定
// 2. 展示头部位移补偿后的预测结果
class DemoWindow : public QWidget {
    Q_OBJECT
public:
    explicit DemoWindow(QWidget *parent = nullptr);

private slots:
    void onCollectAndCalibrate();
    void onPredict();

private:
    CalibrationModel m_model;
    QTextEdit *m_log;
};
