#ifndef OGLWIDGET_H
#define OGLWIDGET_H

#include <QMatrix4x4>
#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QBasicTimer>

class Object3D;
class Transformational;
class Object3DGroup;
class Light;
class Eye;
class SkyBox;
class QOpenGLFramebufferObject;

class OGLWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    OGLWidget(QWidget *parent = nullptr);
    ~OGLWidget();

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void timerEvent(QTimerEvent* event);

    void initShaders();
    void initParallelogram(float width, float height, float depth,
                           QImage *texturemap = nullptr, QImage *normalmap = nullptr);
    void animTimerStop();
    void animTimerStart();
    Object3DGroup* getGroup(const QString& name);
    Object3DGroup* getGroup(int index);
    Object3DGroup* addGroup(const QString &name);
    int delGroup(const QString& name);

private:
    QMatrix4x4 m_ProjectionMatrix;
    QMatrix4x4 m_ProjectionLightMatrix;
    QOpenGLShaderProgram m_ProgramObject;
    QOpenGLShaderProgram m_ProgramSkyBox;
    QOpenGLShaderProgram m_ProgramDepth;
    QVector2D m_MousePosition;
    QQuaternion m_Rotation;
    QVector<Object3D*> m_Objects;
    QHash<QString, Object3DGroup*> m_Groups;
    QBasicTimer m_AnimationTimer;
    Eye* m_Eye;
    SkyBox* m_SkyBox;
    QOpenGLFramebufferObject* m_DepthBuffer;
    QVector<Light*> m_Lights;

    float m_LightPower;                       // мощность основного источника освещения
    float m_ShadowPointCloudFilteringQuality; // качество теней, точность PointCloudFiltering
    int m_ShadowMapSize;                      // качество теней, размер карты теней
    int m_IndexLightShadow;                   // индекс источника света с тенями (основной источник)
    unsigned int m_ShadowTextureSlot;         // слот текстур теней
    bool m_IsDrawShadow;                      // отрисовка теней

    double m_AngleObject;
    double m_AngleGrop1;
    double m_AngleGrop2;
    double m_AngleGrop3;
    double m_AngleGropMain;
    int currentGroup;
};

#endif // OGLWIDGET_H
