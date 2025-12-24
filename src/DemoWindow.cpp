#include "DemoWindow.h"

#include <QDateTime>
#include <QHBoxLayout>

DemoWindow::DemoWindow(QWidget *parent)
    : QWidget(parent)
    , m_log(new QTextEdit(this))
{
    auto *btnCalibrate = new QPushButton(tr("模拟采集并标定"), this);
    auto *btnPredict = new QPushButton(tr("测试预测"), this);

    m_log->setReadOnly(true);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(btnCalibrate);
    layout->addWidget(btnPredict);
    layout->addWidget(m_log);

    connect(btnCalibrate, &QPushButton::clicked, this, &DemoWindow::onCollectAndCalibrate);
    connect(btnPredict, &QPushButton::clicked, this, &DemoWindow::onPredict);

    setWindowTitle(tr("暗瞳眼动校准演示"));
    resize(500, 360);
}

void DemoWindow::onCollectAndCalibrate()
{
    m_log->clear();
    m_log->append(tr("开始采集 9 点标定数据，并模拟头部小范围移动..."));

    m_model = CalibrationModel();

    // 使用 3x3 网格生成屏幕点 (0..1 范围)，并添加头部漂移量模拟真实场景
    const double offset = 0.08; // 头部移动幅度
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            CalibrationModel::Sample s;
            s.screenPoint = QPointF(x / 2.0, y / 2.0); // 屏幕归一化坐标

            // 假设理想情况下眼球相对角膜反射的向量与屏幕点成线性关系
            QPointF idealVector = QPointF((x - 1) * 20.0, (y - 1) * 16.0);

            // 模拟头部位置（z 轴正值代表靠近摄像头）
            QVector3D head(0.05 * (x - 1), 0.05 * (y - 1), 0.2 + 0.01 * (x + y));

            // 将头部位移耦合进观测值：头部左右移动会对瞳孔向量造成偏移
            QPointF observed = idealVector + QPointF(head.x() * 30.0, head.y() * 30.0);

            s.glintCenter = QPointF(320, 240); // 假设角膜反射稳定在中心
            s.pupilCenter = s.glintCenter + observed;
            s.headPosition = head + QVector3D(offset * 0.5 - offset * qrand() / double(RAND_MAX),
                                               offset * 0.5 - offset * qrand() / double(RAND_MAX),
                                               offset * 0.5 - offset * qrand() / double(RAND_MAX));

            m_model.addSample(s);
        }
    }

    if (m_model.calibrate()) {
        m_log->append(tr("标定成功，得到映射系数，可补偿头部位移。"));
    } else {
        m_log->append(tr("标定失败，请增加样本。"));
    }
}

void DemoWindow::onPredict()
{
    if (!m_model.isCalibrated()) {
        m_log->append(tr("请先完成标定。"));
        return;
    }

    // 模拟新的观测：假设目光落在屏幕中心，同时头部向右上移动
    CalibrationModel::Sample target;
    target.screenPoint = QPointF(0.5, 0.5);
    target.glintCenter = QPointF(320, 240);
    target.headPosition = QVector3D(0.04, 0.04, 0.21);

    QPointF idealVector = QPointF(0, 0); // 屏幕中心理想向量
    QPointF observed = idealVector + QPointF(target.headPosition.x() * 30.0,
                                             target.headPosition.y() * 30.0);
    target.pupilCenter = target.glintCenter + observed;

    QPointF predicted = m_model.predict(target.pupilCenter, target.glintCenter, target.headPosition);

    m_log->append(tr("输入头部坐标: (%1, %2, %3)")
                      .arg(target.headPosition.x(), 0, 'f', 3)
                      .arg(target.headPosition.y(), 0, 'f', 3)
                      .arg(target.headPosition.z(), 0, 'f', 3));

    m_log->append(tr("预测屏幕坐标: (%1, %2)")
                      .arg(predicted.x(), 0, 'f', 3)
                      .arg(predicted.y(), 0, 'f', 3));
}
