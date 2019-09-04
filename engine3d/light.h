#ifndef LIGHT_H
#define LIGHT_H

#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>

class Light
{
public:
    enum LightType
    {
        Direct = 0,
        Point = 1,
        Spot = 2
    };

    Light(const LightType &type = Direct);

    const QVector3D DiffuseColor() const;
    void setDiffuseColor(const QVector3D &diffuseColor);

    const QVector3D AmbienceColor() const;
    void setAmbienceColor(const QVector3D &ambienceColor);

    const QVector3D SpecularColor() const;
    void setSpecularColor(const QVector3D &specularColor);

    const QVector4D Position() const;
    void setPosition(const QVector4D &position);

    const QVector4D Direction() const;
    void setDirection(const QVector4D &direction);

    float Cutoff() const;
    void setCutoff(const float& cutoff);

    float Power() const;
    void setPower(const float& power);

    Light::LightType Type() const;
    void setType(const LightType &type);

    const QMatrix4x4 LightMatrix() const;

protected:
    void applyLightMatrix();

private:
    QVector3D m_AmbienceColor;
    QVector3D m_DiffuseColor;
    QVector3D m_SpecularColor;
    QVector4D m_Position;
    QVector4D m_Direction;
    QMatrix4x4 m_LightMatrix;
    float m_Cutoff;
    float m_Power;
    LightType m_Type;
};

#endif // LIGHT_H
