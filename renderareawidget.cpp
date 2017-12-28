#include "renderareawidget.h"
#include<iostream>>
#include <QMouseEvent>
#include "math.h"

const char* vertexShaderSource = R"(
    #version 330 core

    layout( location = 0 ) in vec3 vertexPos;
    uniform mat4 transformMatrix;

    void main()
    {
        gl_Position = transformMatrix * vec4( vertexPos, 1 );
    }
)";


const char* fragmentShaderSource = R"(
    #version 330 core

    uniform vec3 color;
    out vec3 finalColor;

    void main()
    {
        finalColor = color;
    }
)";


RenderAreaWidget::RenderAreaWidget(QWidget* parent)
    : QOpenGLWidget(parent),
      program(nullptr)
{
    previewishappening=false;
    pointspreview.resize(2);
    pressmouse=false;
    ind=-1;

}


RenderAreaWidget::~RenderAreaWidget()
{
    makeCurrent();
    pointsBuffer.destroy();
    previewBuffer.destroy();
    doneCurrent();
    delete program;
}


void RenderAreaWidget::initializeGL()
{
    initializeOpenGLFunctions();

    makeCurrent();

    glViewport(0,0,width(),height());

    //Layout de ponto e linha:
    glEnable(GL_POINT_SMOOTH);
    glEnable( GL_LINE_SMOOTH );
    glLineWidth(1.0f);
    glPointSize(8.0f);

    program = new QOpenGLShaderProgram();
    program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    program->link();

    if (!program->isLinked())
    {
        //TODO: Exibir erro de link e fechar o programa
    }

    program->bind();

    pointsBuffer.create();
    previewBuffer.create();

    proj.ortho(-10.f,10.f,-10.f,10.f,-1.f,1.0f);

    program->setUniformValue("transformMatrix", proj*view);
}


void RenderAreaWidget::paintGL()
{
    program->bind();
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

   if(!points.empty() && pointsBuffer.isCreated())
    {

        pointsBuffer.bind();
        pointsBuffer.allocate( &points[0], (int)points.size()*sizeof(QVector3D) );
        program->enableAttributeArray(0);
        program->setAttributeBuffer(0,GL_FLOAT,0,3,sizeof(QVector3D));


/**********************************************************************************************/

        //Desenha o poligono
        program->setUniformValue("color", QVector3D(1,0,0)); //Vermelho
        glDrawArrays(GL_LINE_STRIP, 0, (int)points.size());

        //Desenha os pontos
        program->setUniformValue("color", QVector3D(1,1,0)); //Amarelo
        glDrawArrays(GL_POINTS, 0, (int)points.size());
        if((!bezier.empty() && pointsBuffer.isCreated() && previewishappening==false) || (!bezier.empty() && pointsBuffer.isCreated() && points.size()>=4) )
        {
           //Desenha a bezier
           pointsBuffer.allocate( &bezier[0], (int)bezier.size()*sizeof(QVector3D) );
           program->setUniformValue("color", QVector3D(1,1,1)); //Branco
           glDrawArrays(GL_LINE_STRIP, 0, (int)bezier.size());
        }
    }
   //Preview
   if(!points.empty() && previewBuffer.isCreated() && points.size()<4)
   {
   previewBuffer.bind();
   previewBuffer.allocate( &pointspreview[0], (int)pointspreview.size()*sizeof(QVector3D) );
   program->enableAttributeArray(0);
   program->setAttributeBuffer(0,GL_FLOAT,0,3,sizeof(QVector3D));
   //desenho da linha
   program->setUniformValue("color", QVector3D(1,0,0)); //Vermelho
   glDrawArrays(GL_LINE_STRIP, 0, (int)pointspreview.size());
   //desenho do ponto
   program->setUniformValue("color", QVector3D(1,1,0)); //Vermelho
   glDrawArrays(GL_POINTS, 0, (int)pointspreview.size()-1);
   //preview bezier
   if(!bezierpreview.empty() && pointsBuffer.isCreated() && previewishappening==true)
   {
      //Desenha a bezier
      previewBuffer.allocate( &bezierpreview[0], (int)bezierpreview.size()*sizeof(QVector3D) );
      program->setUniformValue("color", QVector3D(1,1,1)); //Branco
      glDrawArrays(GL_LINE_STRIP, 0, (int)bezierpreview.size());
   }
   }
}


void RenderAreaWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
}


void RenderAreaWidget::mousePressEvent(QMouseEvent *event)
{
    QVector3D point( event->x(), height()-event->y(), 0 ); // Pegando o ponto que está na tela
    point = point.unproject( view, proj, QRect(0,0,width(),height()));
    point.setZ(0.f);
    pressmouse=true;
    /*se o botão estiver pressionado e não tiver achado um ponto que já tinha sido
     colocado na tela -> Fazer uma busca para ver se o novo clicado
     é igual a um já colocado*/
     if(ind==-1)
        ind=point_search(point);

}


void RenderAreaWidget::mouseMoveEvent(QMouseEvent *event)
{
    QVector3D point( event->x(), height()-event->y(), 0 );
    point = point.unproject( view, proj, QRect(0,0,width(),height()));
    point.setZ(0.f);
    //Emite sinal para que mostre x e y na tela
    QString posText = tr("X = %1, Y = %2").arg( point.x() ).arg(
    point.y() );
    emit updateMousePositionText(posText);
    //Cuida do Preview
    if(points.size()!=0)
    {
    /* Preview reta e pontos: Pega sempre o ultimo ponto que foi clicado e o ponto em que o mouse está*/
    pointspreview[0]=point;
    pointspreview[1]=points.back();
     /*Preview Bezier:pega os pontos que já foram clicados mais o ponto do mouse*/
    pointsbezierpreview=points;
    pointsbezierpreview.push_back(point);
    previewishappening=true;
    }
   if(points.size()<=4)
    {
       bezierpreview=Bezier_curve(pointsbezierpreview); //só faz o preview da bezier se o número de pontos for no máximo 4
    }
    //Cuida da edição
    if(ind!=-1 && pressmouse==true)
    {
        points[ind]=point;
        bezier=Bezier_curve(points);

    }
    update();

}
//Função que cria a bezier
 std::vector< QVector3D > RenderAreaWidget::Bezier_curve( std::vector< QVector3D > pointsb)
{
    std::vector< QVector3D > aux;
    float x, y;
    if(pointsb.size()==2)
    {
        for (float t = 0;t < 1;t = t + 0.01)
        {
            x =(1-t)*pointsb[0].x()+t*pointsb[1].x();
            y = (1-t)*pointsb[0].y()+t*pointsb[1].y();
            QVector3D point( x,y, 0 );
            aux.push_back(point);
        }
    }
    else if(pointsb.size()==3)
    {
    for (float t = 0;t < 1;t = t + 0.01)
    {
        x = (1 - t)*(1 - t)*pointsb[0].x() + 2 * t*(1 - t)*pointsb[1].x() + t*t*pointsb[2].x();
        y = (1 - t)*(1 - t)*pointsb[0].y() + 2 * t*(1 - t)*pointsb[1].y() + t*t*pointsb[2].y();
        QVector3D point( x,y, 0 );
        aux.push_back(point);
    }
    }
    else if(pointsb.size()==4)
    {
    for (float t = 0;t < 1;t = t + 0.01)
    {
        x = (1 - t)*(1 - t)*(1 - t)*pointsb[0].x() + 3 * t*(1 - t)*(1 - t)*pointsb[1].x() + 3 * t*t*(1 - t)*pointsb[2].x() + t*t*t*pointsb[3].x();
        y = (1 - t)*(1 - t)*(1 - t)*pointsb[0].y() + 3 * t*(1 - t)*(1 - t)*pointsb[1].y() + 3 * t*t*(1 - t)*pointsb[2].y() + t*t*t*pointsb[3].y();
        QVector3D point( x,y,0);
        aux.push_back(point);
    }
    }
    return aux;
}
 //Função que faz a busca
int RenderAreaWidget:: point_search(QVector3D point)
{
    double distx,disty;
    for(int i=0;i<points.size();i++)
    {

        distx=fabs(point.x()-points[i].x());
        disty=fabs(point.y()-points[i].y());
        if(distx<0.5 && disty<0.5)
        {

            return i;
        }
    }
    return -1;
}
void RenderAreaWidget::mouseReleaseEvent(QMouseEvent *event)
{
    previewishappening=false;
    QVector3D point( event->x(), height()-event->y(), 0 );
    point = point.unproject( view, proj, QRect(0,0,width(),height()));
    point.setZ(0.f);
    //Apaga com o botão direito o que fez
    if(event->button()==2 && points.size()>0)
    {
         points.pop_back();
    }
    if(event->button()==1)
    {
        //coloca pontos que foram clicados no vetor
        if(points.size()<4 )
        {
          if(ind==-1)
           points.push_back(point);
        }
        //Faz a bezier
        if(points.size()<=4)
        {
           bezier=Bezier_curve(points);
        }
        //Faz o preview quando só tem um ponto
        if(points.size()==1)
        {
            pointspreview[0]=points.back();
            pointspreview[1]=points.back();
        }
    }
    ind=-1;
    pressmouse=false;
     update();
}

