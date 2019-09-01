#include "eye.h"
#include <QOpenGLShaderProgram>

Eye::Eye()
{
    m_Scale = 1.0f;
    m_GlobalTransform.setToIdentity();
}

void Eye::draw(QOpenGLShaderProgram *program, QOpenGLFunctions *functions)
{
    if(functions != nullptr) return;

    udateViewMatrix();

    program->setUniformValue("u_ViewMatrix", m_ViewMatrix);
}

void Eye::rotate(const QQuaternion &r)
{
    m_Rotate = r * m_Rotate;
    udateViewMatrix();
}

void Eye::rotateX(const QQuaternion &r)
{
    m_RotateX = r * m_RotateX;
    m_Rotate = m_RotateX * m_RotateY;
    udateViewMatrix();
}

void Eye::rotateY(const QQuaternion &r)
{
    m_RotateY = r * m_RotateY;
    m_Rotate = m_RotateX * m_RotateY;
    udateViewMatrix();
}

void Eye::translate(const QVector3D &t)
{
    m_Translate += t;
    udateViewMatrix();
}

void Eye::scale(const float &s)
{
    m_Scale *= s;
    udateViewMatrix();
}

void Eye::setGlobalTransform(const QMatrix4x4 &gt)
{
    m_GlobalTransform = gt;
    udateViewMatrix();
}

const QMatrix4x4 Eye::ViewMatrix() const
{
    return m_ViewMatrix;
}

void Eye::udateViewMatrix()
{
    m_ViewMatrix.setToIdentity();
    m_ViewMatrix.translate(m_Translate);
    m_ViewMatrix.rotate(m_Rotate);
    m_ViewMatrix.scale(m_Scale);
    m_ViewMatrix = m_ViewMatrix * m_GlobalTransform.inverted();
}
