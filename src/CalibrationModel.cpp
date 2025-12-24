#include "CalibrationModel.h"

#include <QtMath>
#include <cstring>

CalibrationModel::CalibrationModel()
    : m_calibrated(false)
{
    std::memset(m_weightsX, 0, sizeof(m_weightsX));
    std::memset(m_weightsY, 0, sizeof(m_weightsY));
}

void CalibrationModel::addSample(const CalibrationModel::Sample &sample)
{
    m_samples.push_back(sample);
    m_calibrated = false;
}

bool CalibrationModel::calibrate()
{
    // 需要至少与特征数量相同的样本点以保证可解
    if (m_samples.size() < kFeatureCount) {
        return false;
    }

    double xtx[kFeatureCount][kFeatureCount] = {};
    double xtyX[kFeatureCount] = {};
    double xtyY[kFeatureCount] = {};

    // 将瞳孔-角膜向量与头部坐标拼成特征： [dx, dy, headX, headY, headZ, 1]
    for (const auto &s : m_samples) {
        double feature[kFeatureCount];
        feature[0] = s.pupilCenter.x() - s.glintCenter.x();
        feature[1] = s.pupilCenter.y() - s.glintCenter.y();
        feature[2] = s.headPosition.x();
        feature[3] = s.headPosition.y();
        feature[4] = s.headPosition.z();
        feature[5] = 1.0; // 偏置项，吸收系统误差

        for (int row = 0; row < kFeatureCount; ++row) {
            xtyX[row] += feature[row] * s.screenPoint.x();
            xtyY[row] += feature[row] * s.screenPoint.y();
            for (int col = 0; col < kFeatureCount; ++col) {
                xtx[row][col] += feature[row] * feature[col];
            }
        }
    }

    // 解正规方程 (X^T X) w = X^T y
    double wX[kFeatureCount];
    double wY[kFeatureCount];
    if (!solve6x6(xtx, xtyX, wX) || !solve6x6(xtx, xtyY, wY)) {
        return false;
    }

    std::memcpy(m_weightsX, wX, sizeof(m_weightsX));
    std::memcpy(m_weightsY, wY, sizeof(m_weightsY));
    m_calibrated = true;
    return true;
}

QPointF CalibrationModel::predict(const QPointF &pupilCenter, const QPointF &glintCenter, const QVector3D &headPosition) const
{
    // 若尚未校准，返回原点
    if (!m_calibrated) {
        return {};
    }

    double feature[kFeatureCount];
    feature[0] = pupilCenter.x() - glintCenter.x();
    feature[1] = pupilCenter.y() - glintCenter.y();
    feature[2] = headPosition.x();
    feature[3] = headPosition.y();
    feature[4] = headPosition.z();
    feature[5] = 1.0;

    double x = 0.0;
    double y = 0.0;
    for (int i = 0; i < kFeatureCount; ++i) {
        x += m_weightsX[i] * feature[i];
        y += m_weightsY[i] * feature[i];
    }

    return QPointF(x, y);
}

bool CalibrationModel::solve6x6(const double matrix[6][6], const double rhs[6], double out[6]) const
{
    // 复制矩阵，进行原地高斯消元
    double a[6][7];
    for (int i = 0; i < kFeatureCount; ++i) {
        for (int j = 0; j < kFeatureCount; ++j) {
            a[i][j] = matrix[i][j];
        }
        a[i][6] = rhs[i];
    }

    // 消元过程，加入简单的主元选择以提高稳定性
    for (int col = 0; col < kFeatureCount; ++col) {
        // 选择当前列绝对值最大的行进行交换
        int pivot = col;
        double maxVal = qAbs(a[col][col]);
        for (int row = col + 1; row < kFeatureCount; ++row) {
            double val = qAbs(a[row][col]);
            if (val > maxVal) {
                maxVal = val;
                pivot = row;
            }
        }

        if (qFuzzyIsNull(maxVal)) {
            return false; // 奇异矩阵
        }

        if (pivot != col) {
            for (int j = col; j <= kFeatureCount; ++j) {
                std::swap(a[pivot][j], a[col][j]);
            }
        }

        // 将主元归一化
        double diag = a[col][col];
        for (int j = col; j <= kFeatureCount; ++j) {
            a[col][j] /= diag;
        }

        // 消去其他行
        for (int row = 0; row < kFeatureCount; ++row) {
            if (row == col) continue;
            double factor = a[row][col];
            for (int j = col; j <= kFeatureCount; ++j) {
                a[row][j] -= factor * a[col][j];
            }
        }
    }

    for (int i = 0; i < kFeatureCount; ++i) {
        out[i] = a[i][6];
    }
    return true;
}
