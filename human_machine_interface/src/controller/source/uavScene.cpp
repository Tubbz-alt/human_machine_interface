/*
  qoswidget
  Show the vehicle 3D model.
  @author  Yolanda de la Hoz Simón.
  @date    06-2015
  @version 1.0
*/


#include "../include/uavScene.h"
#include "qt4/QtCore/QDebug"
#include "osgDB/ReadFile"
#include <math.h>


uavScene::uavScene(QWidget *parent, telemetryStateReceiver *telemetryReceiver): QGLWidget(parent)
{
    telemReceiver=telemetryReceiver;
    // Create a view to show the scene with OSG.
    viewer = new osgViewer::Viewer;

    // Create the transformation matrix to show the 3D model, this is the root node.
    transformation = new osg::MatrixTransform;
    auxTransformation = new osg::MatrixTransform;

   // viewer->getCamera()->setClearColor(osg::Vec4(0.2f,0.2f,0.2f,0.0f));

    
    // Load the 3D model, Geodo
    loadedModel = osgDB::readNodeFile("../../../../human_machine_interface/src/view/resources/osg_dataset/pelican2.3ds");
    if(loadedModel.get()==NULL)
      qDebug()<<"NULL";

    // Set camera
    osg::Vec3d eye( 20.0, -20.0, 20.0 );
    osg::Vec3d center( 0.0, 0.0, 0.0 );
    osg::Vec3d up( 0.0, 0.0, 1.0 );


    viewer->getCamera()->setViewMatrixAsLookAt( eye, center, up );


   // Set the 3D mode as child of the transformation.
   auxTransformation->addChild(loadedModel.get());
   auxTransformation->setMatrix(osg::Matrix::rotate( osg::DegreesToRadians(90.0), 1, 0, 0 ));
   transformation->addChild(auxTransformation);

}

void uavScene::initializeGL()
{
    // Use the context of OpenGL created by QGLWidget to OSG.
    // configure the viewport
    window = viewer->setUpViewerAsEmbeddedInWindow(0, 0, width(), height());

    // Set the object type to manipulate the camera
    viewer->setCameraManipulator(new osgGA::TrackballManipulator);
}


void uavScene::resizeGL(int width, int height)
{
    if (window.valid())
    {
        // Adjust the dimensions of the OSG if the widgets change the size
        window->resized(window->getTraits()->x, window->getTraits()->y, width, height);
        window->getEventQueue()->windowResize(window->getTraits()->x, window->getTraits()->y, width, height);
    }

}


void uavScene::paintGL()
{

    // Set a rotation matrix transforamtion to turn the model
    float pitchAngle=osg::DegreesToRadians(telemReceiver->RotationAnglesMsgs.vector.y);
    float rollAngle=osg::DegreesToRadians(telemReceiver->RotationAnglesMsgs.vector.x);

    osg::Matrix pitchMatrix = osg::Matrix::rotate( pitchAngle, 1, 0, 0 );
    osg::Matrix rollMatrix = osg::Matrix::rotate( rollAngle, 0, 1, 0 );

    transformation->setMatrix(pitchMatrix*rollMatrix);

    // Set the root node inside the scene
    viewer->setSceneData(transformation.get());

    // Render the frame
    if (viewer.valid())
        viewer->frame();
}
