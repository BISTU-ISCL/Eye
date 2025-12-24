#pragma once

#include <QPointF>
#include <QVector>
#include <QVector3D>

// 眼动校准模型：使用暗瞳方法，计算瞳孔中心与角膜反射点的相对向量，
// 并结合头部位置补偿，映射到屏幕坐标。
class CalibrationModel {
public:
    struct Sample {
        QPointF pupilCenter;   // 瞳孔中心像素坐标
        QPointF glintCenter;   // 角膜反射点（亮斑）像素坐标
        QVector3D headPosition; // 头部在相机坐标系下的位置（单位可自定）
        QPointF screenPoint;   // 对应的屏幕坐标（归一化或像素）
    };

    CalibrationModel();

    // 增加一条标定样本
    void addSample(const Sample &sample);

    // 执行最小二乘拟合，计算映射参数
    bool calibrate();

    // 根据当前参数预测屏幕坐标
    QPointF predict(const QPointF &pupilCenter,
                    const QPointF &glintCenter,
                    const QVector3D &headPosition) const;

    bool isCalibrated() const { return m_calibrated; }

private:
    static constexpr int kFeatureCount = 6; // dx, dy, headX, headY, headZ, bias

    // 简单的高斯消元解线性方程组，返回是否成功
    bool solve6x6(const double matrix[6][6], const double rhs[6], double out[6]) const;

    double m_weightsX[kFeatureCount];
    double m_weightsY[kFeatureCount];
    QVector<Sample> m_samples;
    bool m_calibrated;
};
