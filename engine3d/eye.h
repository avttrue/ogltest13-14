#ifndef EYE_H
#define EYE_H

#include "transformational.h"

#include <QQuaternion>
#include <QVector3D>
#include <QMatrix4x4>

class Eye : public Transformational
{
public:
    Eye();
    void draw(QOpenGLShaderProgram* program, QOpenGLFunctions*functions = nullptr);
    void rotate(const QQuaternion &r);
    void rotateX(const QQuaternion &r);
    void rotateY(const QQuaternion &r);
    void translate(const QVector3D &t);
    void scale(const float &s);
    void setGlobalTransform(const QMatrix4x4 &gt);
    const QMatrix4x4 ViewMatrix() const;

protected:
    void udateViewMatrix();

private:
    QQuaternion m_Rotate;
    QQuaternion m_RotateX;
    QQuaternion m_RotateY;
    QVector3D m_Translate;
    float m_Scale;
    QMatrix4x4 m_GlobalTransform;
    QMatrix4x4 m_ViewMatrix;

};

#endif // EYE_H
