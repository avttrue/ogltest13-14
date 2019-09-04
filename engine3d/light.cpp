#include "light.h"

#include <QtMath>

Light::Light(const LightType &type):
    m_AmbienceColor(1.0f, 1.0f, 1.0f),
    m_DiffuseColor(1.0f, 1.0f, 1.0f),
    m_SpecularColor(1.0f, 1.0f, 1.0f),
    m_Position(0.0f, 0.0f, 0.0f, 1.0f),
    m_Direction(0.0f, 0.0f, -1.0f, 0.0f),
    m_Cutoff(static_cast<float>(M_PI_2)),
    m_Power(0.5f),
    m_Type(type)
{
    applyLightMatrix();
}


const QVector3D Light::DiffuseColor() const
{
    return m_DiffuseColor;
}

void Light::setDiffuseColor(const QVector3D &diffuseColor)
{
    m_DiffuseColor = diffuseColor;
}

const QVector3D Light::AmbienceColor() const
{
    return m_AmbienceColor;
}

void Light::setAmbienceColor(const QVector3D &ambienceColor)
{
    m_AmbienceColor = ambienceColor;
}

const QVector3D Light::SpecularColor() const
{
    return m_SpecularColor;
}

void Light::setSpecularColor(const QVector3D &specularColor)
{
    m_SpecularColor = specularColor;
}

const QVector4D Light::Position() const
{
    return m_Position;
}

void Light::setPosition(const QVector4D &position)
{
    m_Position = position;
    applyLightMatrix();
}

const QVector4D Light::Direction() const
{
    return m_Direction;
}

void Light::setDirection(const QVector4D &direction)
{
    m_Direction = direction.normalized();
    applyLightMatrix();
}

float Light::Cutoff() const
{
    return m_Cutoff;
}

void Light::setCutoff(const float& cutoff)
{
    m_Cutoff = cutoff;
}

Light::LightType Light::Type() const
{
    return m_Type;
}

void Light::setType(const LightType &type)
{
    m_Type = type;
}

const QMatrix4x4 Light::LightMatrix() const
{
    return m_LightMatrix;
}

void Light::applyLightMatrix()
{
    auto npos = m_Position.normalized();
    m_LightMatrix.setToIdentity();
    m_LightMatrix.lookAt(npos.toVector3D(),
                         (npos + m_Direction).normalized().toVector3D(),
                         QVector3D(m_Direction.x(), m_Direction.z(), -m_Direction.y()));
}

float Light::Power() const
{
    return m_Power;
}

void Light::setPower(const float& power)
{
    m_Power = power;
}
