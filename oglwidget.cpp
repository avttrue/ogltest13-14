#include "oglwidget.h"
#include "engine3d/object3d.h"
#include "engine3d/object3dgroup.h"
#include "engine3d/eye.h"
#include "engine3d/skybox.h"
#include "engine3d/material.h"
#include "engine3d/light.h"

#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QtMath>

OGLWidget::OGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_ShadowTextureSlot = GL_TEXTURE2;  // 0 и 1 текстурные слоты уже заняты

    m_ShadowPointCloudFilteringQuality = 1.0f; // влияет на производительность, можно параметризировать
    m_ShadowMapSize = 1024;// влияет на производительность, можно параметризировать
    m_LightPower = 0.9f; // мощность основного освещения, можно параметризировать

    m_Eye = new Eye;
    m_Eye->translate(QVector3D(0.0f, 0.0f, 0.0f));

    m_IsDrawShadow = true;
    m_IndexLightShadow = 0;

    m_ProjectionLightMatrix.setToIdentity();
    m_ProjectionLightMatrix.ortho(-40.0f, 40.0f, -40.0f, 40.0f, -40.0f, 40.0f); // можно параметризировать

    // основное освещение
    m_Lights.append(new Light(Light::Direct));
    m_Lights.last()->setPosition(QVector4D(20.0f, 15.0f, 55.0f, 1.0f));
    m_Lights.last()->setDirection(QVector4D(-1.0f, -1.0f, -1.0f, 0.0f));

    // дополнительное освещение
    m_Lights.append(new Light(Light::Spot));
    m_Lights.last()->setPosition(QVector4D(15.0f, 10.0f, 10.0f, 1.0f));
    m_Lights.last()->setDirection(QVector4D(-1.0f, -1.0f, -1.0f, 0.0f));
    m_Lights.last()->setDiffuseColor(QVector3D(1.0f, 0.0f, 0.0f));
    m_Lights.last()->setCutoff(20.0f / 180.0f * static_cast<float>(M_PI));

    // дополнительное освещение
    m_Lights.append(new Light(Light::Spot));
    m_Lights.last()->setPosition(QVector4D(20.0f, 10.0f, 5.0f, 1.0f));
    m_Lights.last()->setDirection(QVector4D(-1.0f, -1.0f, -1.0f, 0.0f));
    m_Lights.last()->setDiffuseColor(QVector3D(0.0f, 1.0f, 0.0f));
    m_Lights.last()->setCutoff(15.0f / 180.0f * static_cast<float>(M_PI));

    currentGroup = 0;

    m_AngleObject = 0;
    m_AngleGrop1 = 0;
    m_AngleGrop2 = 0;
    m_AngleGrop3 = 0;
    m_AngleGropMain = 0;
}

OGLWidget::~OGLWidget()
{
    makeCurrent(); // убирает варнинг "QOpenGLTexturePrivate::destroy() called without a current context"
    for(auto o: m_Objects) delete o;
    for(auto o: m_Groups) delete o;
    delete m_Eye;
    delete m_SkyBox;
}

void OGLWidget::initializeGL()
{
    context()->functions()->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    context()->functions()->glEnable(GL_DEPTH_TEST);
    context()->functions()->glEnable(GL_CULL_FACE);

    initShaders();

    float step = 2.0f;

    auto group = addGroup("cube1");
    for(float x = -step; x <= step; x += step)
    {
        for(float y = -step; y <= step; y += step)
        {
            for(float z = -step; z <= step; z += step)
            {
                initParallelogram(1.0f, 1.0f, 1.0f,
                                  new QImage(":/textures/cube1.png"), new QImage(":/textures/cube1_n.png"));
                m_Objects.last()->translate(QVector3D(x, y, z));
                group->add(m_Objects.last());
            }
        }
    }
    group->translate(QVector3D(-5.0f, 0.0f, 0.0f));

    group = addGroup("cube2");
    for(float x = -step; x <= step; x += step)
    {
        for(float y = -step; y <= step; y += step)
        {
            for(float z = -step; z <= step; z += step)
            {
                initParallelogram(1.0f, 1.0f, 1.0f,
                                  new QImage(":/textures/cube2.png"), new QImage(":/textures/cube2_n.png"));
                m_Objects.last()->translate(QVector3D(x, y, z));
                group->add(m_Objects.last());
            }
        }
    }
    group->translate(QVector3D(5.0f, 0.0f, 0.0f));

    group = addGroup("cube3");
    for(float x = -step; x <= step; x += step)
    {
        for(float y = -step; y <= step; y += step)
        {
            for(float z = -step; z <= step; z += step)
            {
                initParallelogram(1.0f, 1.0f, 1.0f,
                                  new QImage(":/textures/cube3.png"), new QImage(":/textures/cube3_n.png"));
                m_Objects.last()->translate(QVector3D(x, y, z));
                group->add(m_Objects.last());
            }
        }
    }
    group->translate(QVector3D(0.0f, 0.0f, -10.0f));

    group = addGroup("All cubes");
    group->add(getGroup("cube1"));
    group->add(getGroup("cube2"));
    group->add(getGroup("cube3"));
    group->translate(QVector3D(0.0f, 15.0f, 0.0f));

    group = addGroup("sphere-cube-cube-pyramid");
    m_Objects.append(new Object3D);
    if(m_Objects.last()->load(":/models/sphere.obj"))
    {
        m_Objects.last()->scale(3.0f);
        m_Objects.last()->translate(QVector3D(-10.0f, 0.0f, 0.0f));
        group->add(m_Objects.last());
    }

    m_Objects.append(new Object3D);
    if(m_Objects.last()->load(":/models/cube.obj"))
    {
        m_Objects.last()->scale(3.0f);
        m_Objects.last()->translate(QVector3D(0.0f, 0.0f, 0.0f));
        group->add(m_Objects.last());
    }

    m_Objects.append(new Object3D);
    if(m_Objects.last()->load(":/models/cube2.obj"))
    {
        m_Objects.last()->scale(3.0f);
        m_Objects.last()->translate(QVector3D(12.0f, 0.0f, 12.0f));
        m_Objects.last()->rotate(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, 45.0f));
        group->add(m_Objects.last());
    }

    m_Objects.append(new Object3D);
    if(m_Objects.last()->load(":/models/pyramid.obj"))
    {
        m_Objects.last()->scale(3.0f);
        m_Objects.last()->translate(QVector3D(10.0f, 0.0f, 0.0f));
        group->add(m_Objects.last());
    }
    group->translate(QVector3D(0.0f, -4.8f, 0.0f));

    group = addGroup("table");
    initParallelogram(60.0, 5.0, 60.0,
                      new QImage(":/textures/cube4.png"), new QImage(":/textures/cube4_n.png"));
    m_Objects.last()->translate(QVector3D(0.0, -10.0, 0.0));
    group->add(m_Objects.last());

    m_SkyBox = new SkyBox(1000.0f,
                          QImage(":/textures/sky/sky_forward.png"),
                          QImage(":/textures/sky/sky_top.png"),
                          QImage(":/textures/sky/sky_bottom.png"),
                          QImage(":/textures/sky/sky_left.png"),
                          QImage(":/textures/sky/sky_right.png"),
                          QImage(":/textures/sky/sky_back.png"));

    m_DepthBuffer = new QOpenGLFramebufferObject(m_ShadowMapSize, m_ShadowMapSize, QOpenGLFramebufferObject::Depth);

    animTimerStart();
}

void OGLWidget::resizeGL(int w, int h)
{
    float aspect = w / (h? static_cast<float>(h) : 1);
    m_ProjectionMatrix.setToIdentity();
    m_ProjectionMatrix.perspective(45, aspect, 0.01f, 1000.0f); // посл. 2 параметра - настройка плоскостей отсечения
}

void OGLWidget::paintGL()
{
    // отрисовка во фреймбуфер
    if(m_IsDrawShadow) // отрисовка теней
    {
        m_DepthBuffer->bind();

        context()->functions()->glViewport(0, 0, m_ShadowMapSize, m_ShadowMapSize);
        context()->functions()->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_ProgramDepth.bind();
        m_ProgramDepth.setUniformValue("u_ProjectionLightMatrix", m_ProjectionLightMatrix);
        m_ProgramDepth.setUniformValue("u_ShadowLightMatrix", m_Lights.at(m_IndexLightShadow)->LightMatrix());
        for(auto o: m_Objects) o->draw(&m_ProgramDepth, context()->functions());
        m_ProgramDepth.release();

        m_DepthBuffer->release();

        GLuint texture = m_DepthBuffer->texture();
        context()->functions()->glActiveTexture(m_ShadowTextureSlot);
        context()->functions()->glBindTexture(GL_TEXTURE_2D, texture);
    }

    // Отрисовка на экран
    context()->functions()->glViewport(0, 0, width(), height());
    context()->functions()->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_ProgramSkyBox.bind();
    m_ProgramSkyBox.setUniformValue("u_ProjectionMatrix", m_ProjectionMatrix);
    m_Eye->draw(&m_ProgramSkyBox);
    m_SkyBox->draw(&m_ProgramSkyBox, context()->functions());
    m_ProgramSkyBox.release();

    m_ProgramObject.bind();
    m_ProgramObject.setUniformValue("u_IsDrawShadow", m_IsDrawShadow);
    m_ProgramObject.setUniformValue("u_ShadowMap", m_ShadowTextureSlot - GL_TEXTURE0);
    m_ProgramObject.setUniformValue("u_ShadowMapSize", static_cast<float>(m_ShadowMapSize));
    m_ProgramObject.setUniformValue("u_ShadowPointCloudFilteringQuality", m_ShadowPointCloudFilteringQuality);
    m_ProgramObject.setUniformValue("u_ProjectionMatrix", m_ProjectionMatrix);
    m_ProgramObject.setUniformValue("u_ProjectionLightMatrix", m_ProjectionLightMatrix);
    m_ProgramObject.setUniformValue("u_CountLights", m_Lights.size());
    m_ProgramObject.setUniformValue("u_IndexLightShadow", m_IndexLightShadow);
    m_ProgramObject.setUniformValue("u_ShadowLightMatrix", m_Lights.at(m_IndexLightShadow)->LightMatrix());
    for(int i = 0; i < m_Lights.size(); i++)
    {
        m_ProgramObject.setUniformValue(QString("u_LightProperty[%1].AmbienceColor").arg(i).toLatin1().data(), m_Lights.at(i)->AmbienceColor());
        m_ProgramObject.setUniformValue(QString("u_LightProperty[%1].DiffuseColor").arg(i).toLatin1().data(), m_Lights.at(i)->DiffuseColor());
        m_ProgramObject.setUniformValue(QString("u_LightProperty[%1].SpecularColor").arg(i).toLatin1().data(), m_Lights.at(i)->SpecularColor());
        m_ProgramObject.setUniformValue(QString("u_LightProperty[%1].ReflectionColor").arg(i).toLatin1().data(), m_Lights.at(i)->ReflectionColor());
        m_ProgramObject.setUniformValue(QString("u_LightProperty[%1].Position").arg(i).toLatin1().data(), m_Lights.at(i)->Position());
        m_ProgramObject.setUniformValue(QString("u_LightProperty[%1].Direction").arg(i).toLatin1().data(), m_Lights.at(i)->Direction());
        m_ProgramObject.setUniformValue(QString("u_LightProperty[%1].Cutoff").arg(i).toLatin1().data(), m_Lights.at(i)->Cutoff());
        m_ProgramObject.setUniformValue(QString("u_LightProperty[%1].Power").arg(i).toLatin1().data(), m_Lights.at(i)->Power());
        m_ProgramObject.setUniformValue(QString("u_LightProperty[%1].Type").arg(i).toLatin1().data(), m_Lights.at(i)->Type());
    }

    m_Eye->draw(&m_ProgramObject);
    for(auto o: m_Objects) o->draw(&m_ProgramObject, context()->functions());
    m_ProgramObject.release();
}

void OGLWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() == Qt::LeftButton)
    {
        m_MousePosition = QVector2D(event->localPos());
    }

    event->accept();
}

void OGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    switch (event->buttons())
    {
    case Qt::LeftButton:
    {
        QVector2D diffpos = QVector2D(event->localPos()) - m_MousePosition;
        m_MousePosition = QVector2D(event->localPos());

        //        float angle = diffpos.length() / 2.0f;
        //        QVector3D axis = QVector3D(diffpos.y(), diffpos.x(), 0.0f);
        //        m_Eye->rotate(QQuaternion::fromAxisAndAngle(axis, angle));

        float angleX = diffpos.y() / 2.0f;
        float angleY = diffpos.x() / 2.0f;
        m_Eye->rotateX(QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, angleX));
        m_Eye->rotateY(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, angleY));
        update();
    }

    }
    event->accept();
}

void OGLWidget::wheelEvent(QWheelEvent *event)
{
    if(event->modifiers() & Qt::ShiftModifier)
    {
        if(event->delta() > 0 && m_LightPower < 1) m_LightPower += 0.05f;
        else if(event->delta() < 0 && m_LightPower >= 0) m_LightPower -= 0.05f;

        m_Lights.first()->setPower(m_LightPower);
        qDebug() << "Main light power:" << m_LightPower;
        return;
    }

    float mod = event->modifiers() & Qt::ControlModifier ? 3.0f : 1.0f;
    if(event->delta() > 0) m_Eye->translate(QVector3D(0.0f, 0.0f, 0.25f * mod));
    else if(event->delta() < 0) m_Eye->translate(QVector3D(0.0f, 0.0f, -0.25f * mod));

    update();
}

void OGLWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Q:
    {
        close();
        break;
    }
    case Qt::Key_S:
    {
        m_IsDrawShadow = !m_IsDrawShadow;
        break;
    }
    case Qt::Key_Tab:
    {
        currentGroup >= m_Groups.size() - 1 ? currentGroup = 0 : currentGroup++;

        for(auto g: m_Groups) g->del(m_Eye);

        auto g = getGroup(currentGroup); if(!g) return;
        g->add(m_Eye);
        qDebug() << "Current group:" << g->Name();
        break;
    }
    case Qt::Key_Delete:
    {
        for(auto g: m_Groups) g->del(m_Eye);
        break;
    }
    case Qt::Key_Escape:
    {
        for(auto g: m_Groups) g->del(m_Eye);
        QMatrix4x4 m; m.setToIdentity();
        m_Eye->setGlobalTransform(m);
        qDebug() << "Current group: None";
        break;
    }
    case Qt::Key_Space:
    {
        m_AnimationTimer.isActive() ? animTimerStop() : animTimerStart();
        break;
    }
    }
}

void OGLWidget::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event)

    //m_LightRotateY += 30 * static_cast<float>(qSin(M_PI / 360.0));
    //applyMainLight();

    Object3DGroup* g = getGroup("cube1");
    if(g)
    {
        for(int i = 0; i < g->size(); i++)
        {
            if(i % 2 == 0)
            {
                g->at(i)->rotate(QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, static_cast<float>(qSin(m_AngleObject))));
                g->at(i)->rotate(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, static_cast<float>(qCos(m_AngleObject))));
            }
            else
            {
                g->at(i)->rotate(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, static_cast<float>(qSin(m_AngleObject))));
                g->at(i)->rotate(QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, static_cast<float>(qCos(m_AngleObject))));
            }
        }
        g->rotate(QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, static_cast<float>(qSin(m_AngleGrop1))));
        g->rotate(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, static_cast<float>(-qSin(m_AngleGrop1))));
    }

    g = getGroup("cube2");
    if(g)
    {
        for(int i = 0; i < g->size(); i++)
        {
            if(i % 2 == 0)
            {
                g->at(i)->rotate(QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, static_cast<float>(qSin(m_AngleObject))));
                g->at(i)->rotate(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, static_cast<float>(qCos(m_AngleObject))));
            }
            else
            {
                g->at(i)->rotate(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, static_cast<float>(qSin(m_AngleObject))));
                g->at(i)->rotate(QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, static_cast<float>(qCos(m_AngleObject))));
            }
        }
        g->rotate(QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, static_cast<float>(qSin(m_AngleGrop1))));
        g->rotate(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, static_cast<float>(-qSin(m_AngleGrop1))));
    }

    g = getGroup("cube3");
    if(g)
    {
        for(int i = 0; i < g->size(); i++)
        {
            if(i % 2 == 0)
            {
                g->at(i)->rotate(QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, static_cast<float>(qSin(m_AngleObject))));
                g->at(i)->rotate(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, static_cast<float>(qCos(m_AngleObject))));
            }
            else
            {
                g->at(i)->rotate(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, static_cast<float>(qSin(m_AngleObject))));
                g->at(i)->rotate(QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, static_cast<float>(qCos(m_AngleObject))));
            }
        }
        g->rotate(QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, static_cast<float>(qSin(m_AngleGrop1))));
        g->rotate(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, static_cast<float>(-qSin(m_AngleGrop1))));
    }

    g = getGroup("All cubes");
    if(g)
    {
        g->rotate(QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, static_cast<float>(-qSin(m_AngleGropMain))));
        g->rotate(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, static_cast<float>(-qCos(m_AngleGropMain))));
    }

    m_AngleObject += M_PI / 360.0;
    m_AngleGrop1 += M_PI / 270.0;
    m_AngleGrop2 -= M_PI / 270.0;
    m_AngleGrop3 -= M_PI / 270.0;
    m_AngleGropMain += M_PI / 720.0;

    update();
}

void OGLWidget::initShaders()
{
    if(! m_ProgramObject.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/object.vsh"))
        close();
    if(! m_ProgramObject.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/object.fsh"))
        close();
    if(! m_ProgramObject.link()) close();

    if(! m_ProgramSkyBox.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/skybox.vsh"))
        close();
    if(! m_ProgramSkyBox.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/skybox.fsh"))
        close();
    if(! m_ProgramSkyBox.link()) close();

    if(! m_ProgramDepth.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/depth.vsh"))
        close();
    if(! m_ProgramDepth.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/depth.fsh"))
        close();
    if(! m_ProgramDepth.link()) close();
}

void OGLWidget::initParallelogram(float width, float height, float depth,
                                  QImage *texturemap, QImage *normalmap)
{
    float width_div_2 = width / 2.0f;
    float height_div_2 = height / 2.0f;
    float depth_div_2 = depth / 2.0f;

    QVector<VertexData> vertexes;
    vertexes.append(VertexData(QVector3D(-width_div_2, height_div_2, depth_div_2), QVector2D(0.0f, 1.0f), QVector3D(0.0f, 0.0f, 1.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, -height_div_2, depth_div_2), QVector2D(0.0f, 0.0f), QVector3D(0.0f, 0.0f, 1.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, height_div_2, depth_div_2), QVector2D(1.0f, 1.0f), QVector3D(0.0f, 0.0f, 1.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, height_div_2, depth_div_2), QVector2D(1.0f, 1.0f), QVector3D(0.0f, 0.0f, 1.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, -height_div_2, depth_div_2), QVector2D(0.0f, 0.0f), QVector3D(0.0f, 0.0f, 1.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, -height_div_2, depth_div_2), QVector2D(1.0f, 0.0f), QVector3D(0.0f, 0.0f, 1.0f)));

    vertexes.append(VertexData(QVector3D(width_div_2, height_div_2, depth_div_2), QVector2D(0.0f, 1.0f), QVector3D(1.0f, 0.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, -height_div_2, depth_div_2), QVector2D(0.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, height_div_2, -depth_div_2), QVector2D(1.0f, 1.0f), QVector3D(1.0f, 0.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, height_div_2, -depth_div_2), QVector2D(1.0f, 1.0f), QVector3D(1.0f, 0.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, -height_div_2, depth_div_2), QVector2D(0.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, -height_div_2, -depth_div_2), QVector2D(1.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f)));

    vertexes.append(VertexData(QVector3D(width_div_2, height_div_2, depth_div_2), QVector2D(0.0f, 1.0f), QVector3D(0.0f, 1.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, height_div_2, -depth_div_2), QVector2D(0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, height_div_2, depth_div_2), QVector2D(1.0f, 1.0f), QVector3D(0.0f, 1.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, height_div_2, depth_div_2), QVector2D(1.0f, 1.0f), QVector3D(0.0f, 1.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, height_div_2, -depth_div_2), QVector2D(0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, height_div_2, -depth_div_2), QVector2D(1.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f)));

    vertexes.append(VertexData(QVector3D(width_div_2, height_div_2, -depth_div_2), QVector2D(0.0f, 1.0f), QVector3D(0.0f, 0.0f, -1.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, -height_div_2, -depth_div_2), QVector2D(0.0f, 0.0f), QVector3D(0.0f, 0.0f, -1.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, height_div_2, -depth_div_2), QVector2D(1.0f, 1.0f), QVector3D(0.0f, 0.0f, -1.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, height_div_2, -depth_div_2), QVector2D(1.0f, 1.0f), QVector3D(0.0f, 0.0f, -1.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, -height_div_2, -depth_div_2), QVector2D(0.0f, 0.0f), QVector3D(0.0f, 0.0f, -1.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, -height_div_2, -depth_div_2), QVector2D(1.0f, 0.0f), QVector3D(0.0f, 0.0f, -1.0f)));

    vertexes.append(VertexData(QVector3D(-width_div_2, height_div_2, depth_div_2), QVector2D(0.0f, 1.0f), QVector3D(-1.0f, 0.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, height_div_2, -depth_div_2), QVector2D(0.0f, 0.0f), QVector3D(-1.0f, 0.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, -height_div_2, depth_div_2), QVector2D(1.0f, 1.0f), QVector3D(-1.0f, 0.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, -height_div_2, depth_div_2), QVector2D(1.0f, 1.0f), QVector3D(-1.0f, 0.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, height_div_2, -depth_div_2), QVector2D(0.0f, 0.0f), QVector3D(-1.0f, 0.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, -height_div_2, -depth_div_2), QVector2D(1.0f, 0.0f), QVector3D(-1.0f, 0.0f, 0.0f)));

    vertexes.append(VertexData(QVector3D(-width_div_2, -height_div_2, depth_div_2), QVector2D(0.0f, 1.0f), QVector3D(0.0f, -1.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, -height_div_2, -depth_div_2), QVector2D(0.0f, 0.0f), QVector3D(0.0f, -1.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, -height_div_2, depth_div_2), QVector2D(1.0f, 1.0f), QVector3D(0.0f, -1.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, -height_div_2, depth_div_2), QVector2D(1.0f, 1.0f), QVector3D(0.0f, -1.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, -height_div_2, -depth_div_2), QVector2D(0.0f, 0.0f), QVector3D(0.0f, -1.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, -height_div_2, -depth_div_2), QVector2D(1.0f, 0.0f), QVector3D(0.0f, -1.0f, 0.0f)));

    QVector<GLuint> indexes;
    for(GLuint i = 0; i < 36; ++i) indexes.append(i);

    Material* mtl = new Material;
    if (texturemap) mtl->setDiffuseMap(*texturemap);
    if (normalmap) mtl->setNormalMap(*normalmap);
    mtl->setShines(96.0f);
    mtl->setDiffuseColor(QVector3D(1.0f, 1.0f, 1.0f));
    mtl->setAmbienceColor(QVector3D(1.0f, 1.0f, 1.0f));
    mtl->setSpecularColor(QVector3D(1.0f, 1.0f, 1.0f));

    Object3D* eo3d = new Object3D;
    eo3d->calculateTBN(vertexes);
    eo3d->add(new Object3DElement(vertexes, indexes, mtl));
    m_Objects.append(eo3d);
}

void OGLWidget::animTimerStop()
{
    if(m_AnimationTimer.isActive()) m_AnimationTimer.stop();
}

void OGLWidget::animTimerStart()
{
    if(!m_AnimationTimer.isActive()) m_AnimationTimer.start(30, this);
}

Object3DGroup *OGLWidget::getGroup(int index)
{
    if(index < 0 || m_Groups.size() <= index) return nullptr;

    auto list = m_Groups.keys();
    list.sort(Qt::CaseInsensitive);
    auto name = list.at(index);

    return m_Groups.value(name, nullptr);
}

Object3DGroup* OGLWidget::getGroup(const QString& name)
{
    return m_Groups.value(name, nullptr);
}

Object3DGroup* OGLWidget::addGroup(const QString& name)
{
    auto result = new Object3DGroup(name);
    m_Groups.insert(name, result);
    return result;

}

int OGLWidget::delGroup(const QString &name)
{
    return m_Groups.remove(name);
}

