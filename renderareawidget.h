
#ifndef RENDERAREAWIDGET_H
#define RENDERAREAWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

#include <vector>


/******************************************************************************************************************
* - O programa faz uma Bezier quadrática quando 3 pontos são clicados
* - Quando 4 pontos são clicados, o programa faz uma Bezier cúbica
* - Os pontos podem ser editados clicando com o botão esquerdo do mouse e arrastando.
* - Com o botão direito, o programa volta e vai apagando o que já fez.
* ******************************************************************************************************************/
class RenderAreaWidget
    : public QOpenGLWidget
    , protected QOpenGLFunctions
{
Q_OBJECT

public:
    explicit RenderAreaWidget(QWidget* parent = 0);
    ~RenderAreaWidget();

    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    std::vector< QVector3D > Bezier_curve(std::vector< QVector3D > pointsb);

    int  point_search(QVector3D point);
signals:
 void updateMousePositionText(const QString& message);

private:
    std::vector< QVector3D > points; //vetor de pontos que são clicados
    std::vector< QVector3D > bezier;//vetor com pontos que constituem a Bezier
    std::vector< QVector3D > pointspreview; //vetor com pontos do preview
    std::vector< QVector3D > bezierpreview; //vetor com preview da Bezier
    std::vector< QVector3D > pointsbezierpreview; //vetor de pontos auxiliar para fazer Bezier
    QOpenGLShaderProgram* program;
    QOpenGLBuffer pointsBuffer; //Buffer para os pontos, as retas e a Bezier
    QOpenGLBuffer previewBuffer; //Buffer para todos os previews
    bool previewishappening; //variável que avisa se o preview está acontecendo ou não
    bool pressmouse; //flag pra dizer se botão tá pressionado ou não
    int ind; //indice do ponto encontrado na busca

    QMatrix4x4 view;
    QMatrix4x4 proj;
};

#endif // RENDERAREAWIDGET_H
