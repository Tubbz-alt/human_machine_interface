 /*
  Connection - Intializes ROS threads to send and receive packages 
  @author  Yolanda de la Hoz Simón
  @date    05-2015
  @version 2.0
*/

/*****************************************************************************
** Includes
*****************************************************************************/
#include "../include/connection.h"
#include <qt4/Qt/qsettings.h>
#include "../../view/ui/ui_connection.h"
/*****************************************************************************
** Implementation
*****************************************************************************/

Connection::Connection(QWidget *parent,int argc, char** argv):
    init_argc(argc),
    init_argv(argv),
    QDialog(parent),
    ui(new Ui::connection)
{
	ui->setupUi(this);

    telemetry_receiver= new TelemetryStateReceiver();
    odometry_receiver= new OdometryStateReceiver();
    img_receiver = new ImagesReceiver();
    usercommander= new UserCommander();
    graph_receiver= new RosGraphReceiver();
   
    connect_status=false;
    ReadSettings();
    connect(ui->connect_button,SIGNAL(clicked(bool)),this, SLOT(onButton_connect_clicked(bool)));
}


/*
 * These triggers whenever the button is clicked, regardless of whether it
 * is already checked or not.
 */
void Connection::onButton_connect_clicked(bool check)
{
    std::string master;
    std::string host;
    if ( ui->checkbox_use_environment->isChecked() ) {
        if(!this->init()){
            connect_status=false;
         }else{
            connect_status=true;
            ui->connect_button->setEnabled(false);
         }
    }else{
        master=ui->line_edit_master->text().toStdString();
        host=ui->line_edit_host->text().toStdString();
        if(!this->init(master,host)){
            connect_status=false;
        }else{
            ui->connect_button->setEnabled(false);
            ui->line_edit_master->setReadOnly(true);
            ui->line_edit_host->setReadOnly(true);
            connect_status=true;
        }
    }
   Q_EMIT connectionEstablish();
}

void Connection::on_checkbox_use_environment_stateChanged(int state)
{
    bool enabled;
    if ( state == 0 )
        enabled = true;
    else
        enabled = false;

    ui->line_edit_master->setEnabled(enabled);
    ui->line_edit_host->setEnabled(enabled);
}

bool Connection::initInCommon(){

    if ( ! ros::master::check() ) // Check if roscore has been initialized.
      return false;

    ros::start(); // explicitly call to ros start
    ros::NodeHandle n;

    if(ros::this_node::getNamespace().compare(" /"))
        rosnamespace.append("/drone1");//default namespace
    else
        rosnamespace.append(ros::this_node::getNamespace());

    std::cout << "Namespace con ros::this_node::getNamespace(): " << ros::this_node::getNamespace() << std::endl;


    // Start query
    telemetry_receiver->openSubscriptions(n, rosnamespace);
    odometry_receiver->openSubscriptions(n, rosnamespace);
    img_receiver->openSubscriptions(n, rosnamespace);
    graph_receiver->openSubscriptions(n, rosnamespace);
    // Start command
    usercommander->openPublications(n, rosnamespace);


    if(!Connection::readyForConnect())
     return false;

    std::thread thr(&Connection::spinnerThread, this);
    std::swap(thr, connection_admin_thread);

    return true;
}

bool Connection::init()
{
    ros::init(init_argc,init_argv,"human_machine_interface");// ros node started.

    return Connection::initInCommon();
}

bool Connection::init(const std::string &master_url, const std::string &host_url)
{

    std::map<std::string,std::string> remappings;
    remappings["__master"] = master_url;
    remappings["__hostname"] = host_url;

    ros::init(remappings,"human_machine_interface");

    return Connection::initInCommon();
}

bool Connection::readyForConnect()
{
    if(!telemetry_receiver->ready() || !odometry_receiver->ready() || !img_receiver->ready() || !graph_receiver->ready() || !usercommander->ready())
        return false;
    return true;
}

void Connection::spinnerThread()
{
    ros::spin();
    if(ros::isStarted()) {
      ros::shutdown(); // Kill all open subscriptions, publications, service calls, and service servers.
      ros::waitForShutdown();
    }
    std::cout << "Ros shutdown, proceeding to close the gui." << std::endl;
    Q_EMIT rosShutdown(); // used to signal the gui for a shutdown

}

void Connection::ReadSettings()
{
    QSettings settings("Human Machine Interface - QSettings", node_name.c_str());
    QString master_url = settings.value("master_url",QString("http://192.168.1.2:11311/")).toString();
    QString host_url = settings.value("host_url", QString("192.168.1.3")).toString();
    ui->line_edit_master->setText(master_url);
    ui->line_edit_host->setText(host_url);
    bool remember = settings.value("remember_settings", false).toBool();
    ui->checkbox_remember_settings->setChecked(remember);
    bool checked = settings.value("use_environment_variables", false).toBool();
    ui->checkbox_use_environment->setChecked(checked);
    if ( checked ) {
        ui->line_edit_master->setEnabled(false);
        ui->line_edit_host->setEnabled(false);
    }
}

void Connection::WriteSettings()
{
    QSettings settings("Human Machine Interface - QSettings", node_name.c_str());
    settings.setValue("master_url",ui->line_edit_master->text());
    settings.setValue("host_url",ui->line_edit_host->text());
    settings.setValue("use_environment_variables",QVariant(ui->checkbox_use_environment->isChecked()));
    settings.setValue("remember_settings",QVariant(ui->checkbox_remember_settings->isChecked()));
}

void Connection::close()
{
    this->~Connection();
}

Connection::~Connection()
{
    delete ui;
}

