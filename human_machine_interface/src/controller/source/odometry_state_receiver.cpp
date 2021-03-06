/*
  OdometryStateReceiver
  Launch a ROS node to subscribe topics.
  @author  Yolanda de la Hoz Simón
  @date    03-2015
  @version 1.0
*/

/*****************************************************************************
** Includes
*****************************************************************************/

#include <ros/ros.h>
#include <ros/network.h>
#include <string>
#include <std_msgs/String.h>
#include <sstream>
#include "../include/odometry_state_receiver.h"


/*****************************************************************************
** Implementation
*****************************************************************************/

OdometryStateReceiver::OdometryStateReceiver(){
     subscriptions_complete = false;
}


void OdometryStateReceiver::openSubscriptions(ros::NodeHandle nodeHandle, std::string rosnamespace){

    readParams(nodeHandle);

    DroneGMREstimatedPoseSubs=nodeHandle.subscribe(rosnamespace + "/" + drone_trajectory_controller_pose_subscription_gmr, 1, &OdometryStateReceiver::droneGMREstimatedPoseCallback, this); //EstimatedPose_droneGMR_wrt_GFF
    DroneGMREstimatedSpeedSubs=nodeHandle.subscribe(rosnamespace + "/" +  drone_trajectory_controller_speeds_subscription_gmr, 1, &OdometryStateReceiver::droneGMREstimatedSpeedCallback, this);//EstimatedSpeed_droneGMR_wrt_GFF
    DroneSOEstimatedPoseSubs=nodeHandle.subscribe(rosnamespace + "/" + drone_state_estimator_interface_pose_subscription_lmrt, 1, &OdometryStateReceiver::droneSOEstimatedPoseCallback, this); //SOEstimatedPose
    DroneSOEstimatedSpeedsSubs=nodeHandle.subscribe(rosnamespace + "/" +  drone_state_estimator_interface_speeds_subscription_lmrt, 1, &OdometryStateReceiver::droneSOEstimatedSpeedsCallback, this);//SOEstimatedSpeeds


    // Topic communications droneArucoEye
    DroneArucoEstimatedPoseSubs=nodeHandle.subscribe(rosnamespace + "/" + drone_trajectory_planner_pose_subscription, 1, &OdometryStateReceiver::droneArucoEstimatedPoseCallback, this); //ArucoSlam_EstimatedPose
    DroneArucoEstimatedSpeedSubs=nodeHandle.subscribe(rosnamespace + "/" +  drone_trajectory_controller_speeds_subscription_gmr_aruco, 1, &OdometryStateReceiver::droneArucoEstimatedSpeedCallback, this);//ArucoSlam_EstimatedSpeeds
    DroneArucoEyeObservationSubs=nodeHandle.subscribe(rosnamespace + "/" +  drone_logger_aruco_eye_observationvec_list, 1, &OdometryStateReceiver::droneArucoEyeObservationCallback, this);


    // Topic communications controller
    // Controller references (rebroadcasts): control mode and position, speed and trajectory references
    DroneTrajectoryPositionSubs=nodeHandle.subscribe(rosnamespace + "/" + drone_logger_position_ref_rebroadcast_subscription, 1, &OdometryStateReceiver::dronePoseCallback, this);
    DroneTrajectorySpeedsSubs=nodeHandle.subscribe(rosnamespace + "/" + drone_logger_speed_ref_rebroadcast_subscription, 1, &OdometryStateReceiver::droneSpeedsCallback, this);

    subscriptions_complete = true;
//    real_time=ros;
}

bool OdometryStateReceiver::ready() {
    if (!subscriptions_complete)
        return false;
    return true; //Used this way instead of "return subscriptions_complete" due to preserve add more conditions
}

OdometryStateReceiver::~OdometryStateReceiver() {}


void OdometryStateReceiver::readParams(ros::NodeHandle nodeHandle){

    if (!nodeHandle.getParam("drone_trajectory_controller_pose_subscription_gmr", drone_trajectory_controller_pose_subscription_gmr))
      drone_trajectory_controller_pose_subscription_gmr = "ArucoSlam_EstimatedPose";

    if (!nodeHandle.getParam("drone_trajectory_controller_speeds_subscription_gmr", drone_trajectory_controller_speeds_subscription_gmr))
      drone_trajectory_controller_speeds_subscription_gmr = "EstimatedSpeed_droneGMR_wrt_GFF";
    
    if (!nodeHandle.getParam("drone_state_estimator_interface_pose_subscription_lmrt", drone_state_estimator_interface_pose_subscription_lmrt))
      drone_state_estimator_interface_pose_subscription_lmrt = "SOEstimatedPose";

    if (!nodeHandle.getParam("drone_state_estimator_interface_speeds_subscription_lmrt", drone_state_estimator_interface_speeds_subscription_lmrt))
      drone_state_estimator_interface_speeds_subscription_lmrt = "SOEstimatedSpeeds";
    
    if (!nodeHandle.getParam("drone_trajectory_planner_pose_subscription", drone_trajectory_planner_pose_subscription))
      drone_trajectory_planner_pose_subscription = "ArucoSlam_EstimatedPose";

    if (!nodeHandle.getParam("drone_trajectory_controller_speeds_subscription_gmr_aruco", drone_trajectory_controller_speeds_subscription_gmr_aruco))
      drone_trajectory_controller_speeds_subscription_gmr_aruco = "ArucoSlam_EstimatedSpeeds";

    if (!nodeHandle.getParam("drone_logger_aruco_eye_observationvec_list", drone_logger_aruco_eye_observationvec_list))
      drone_logger_aruco_eye_observationvec_list = "arucoObservationl";

    if (!nodeHandle.getParam("drone_logger_position_ref_rebroadcast_subscription", drone_logger_position_ref_rebroadcast_subscription))
      drone_logger_position_ref_rebroadcast_subscription = "trajectoryControllerPositionReferencesRebroadcast";
    
    if (!nodeHandle.getParam("drone_logger_speed_ref_rebroadcast_subscription", drone_logger_speed_ref_rebroadcast_subscription))
      drone_logger_speed_ref_rebroadcast_subscription = "trajectoryControllerSpeedReferencesRebroadcast";


}

void OdometryStateReceiver::droneArucoEstimatedPoseCallback(const droneMsgsROS::dronePose::ConstPtr &msg)
{
    drone_pose_msgs=*msg;
    Q_EMIT parameterReceived();
    Q_EMIT updateStatus();
    //printDroneArucoEstimatedPoseCallback(msg);
    return;

}

void OdometryStateReceiver::printDroneArucoEstimatedPoseCallback(const droneMsgsROS::dronePose::ConstPtr &msg)
{
    log(Info,std::string("Received pitch from ArucoSlam_EstimatedPose: ")+ boost::lexical_cast<std::string>(msg->pitch));
    log(Info,std::string("Received roll from ArucoSlam_EstimatedPose: ")+ boost::lexical_cast<std::string>(msg->roll));
    log(Info,std::string("Received yaw from ArucoSlam_EstimatedPose: ")+ boost::lexical_cast<std::string>(msg->yaw));
    log(Info,std::string("Received yaw from ArucoSlam_EstimatedPose: ")+ boost::lexical_cast<std::string>(msg->x));
    log(Info,std::string("Received pos.y from ArucoSlam_EstimatedPose: ")+ boost::lexical_cast<std::string>(msg->y));
    log(Info,std::string("Received pos.z from ArucoSlam_EstimatedPose: ")+ boost::lexical_cast<std::string>(msg->z));
    return;
}

void OdometryStateReceiver::droneArucoEstimatedSpeedCallback(const droneMsgsROS::droneSpeeds::ConstPtr &msg)
{
    drone_speeds_msgs=*msg;
    Q_EMIT parameterReceived();
    Q_EMIT updateStatus();
    //printDroneArucoEstimatedSpeedCallback(msg);
    return;

}

void OdometryStateReceiver::printDroneArucoEstimatedSpeedCallback(const droneMsgsROS::droneSpeeds::ConstPtr &msg)
{
    log(Info,std::string("Received dx from ArucoSlam_EstimatedSpeeds: [%")+ boost::lexical_cast<std::string>(msg->dx));
    log(Info,std::string("Received dy from ArucoSlam_EstimatedSpeeds: ")+ boost::lexical_cast<std::string>(msg->dy));
    log(Info,std::string("Received dz from ArucoSlam_EstimatedSpeeds: ")+ boost::lexical_cast<std::string>(msg->dz));
    log(Info,std::string("Received dyaw from ArucoSlam_EstimatedSpeeds: ")+ boost::lexical_cast<std::string>(msg->dyaw));
    log(Info,std::string("Received dpitch from ArucoSlam_EstimatedSpeeds: ")+ boost::lexical_cast<std::string>(msg->dpitch));
    log(Info,std::string("Received droll from ArucoSlam_EstimatedSpeeds: ")+ boost::lexical_cast<std::string>(msg->droll));
    return;

}


void OdometryStateReceiver::droneGMREstimatedPoseCallback(const droneMsgsROS::dronePose::ConstPtr &msg)
{
    drone_pose_msgs=*msg;
    Q_EMIT parameterReceived();
    Q_EMIT updateStatus();
    //printDroneGMREstimatedPoseCallback(msg);
    return;
}

void OdometryStateReceiver::printDroneGMREstimatedPoseCallback(const droneMsgsROS::dronePose::ConstPtr &msg)
{
    log(Info,std::string("Received dx from EstimatedSpeed_droneGMR_wrt_GFF: ")+ boost::lexical_cast<std::string>(msg->pitch));
    log(Info,std::string("Received dy from EstimatedSpeed_droneGMR_wrt_GFF: ")+ boost::lexical_cast<std::string>(msg->roll));
    log(Info,std::string("Received dz from EstimatedSpeed_droneGMR_wrt_GFF:")+ boost::lexical_cast<std::string>(msg->yaw));
    log(Info,std::string("Received dyaw from EstimatedSpeed_droneGMR_wrt_GFF: ")+ boost::lexical_cast<std::string>(msg->x));
    log(Info,std::string("Received dpitch from EstimatedSpeed_droneGMR_wrt_GFF:  ")+ boost::lexical_cast<std::string>(msg->y));
    log(Info,std::string("Received droll from EstimatedSpeed_droneGMR_wrt_GFF: ")+ boost::lexical_cast<std::string>(msg->z));
    return;
}

void OdometryStateReceiver::droneGMREstimatedSpeedCallback(const droneMsgsROS::droneSpeeds::ConstPtr &msg)
{
    drone_speeds_msgs=*msg;
    Q_EMIT parameterReceived();
    Q_EMIT updateStatus();
    //printDroneGMREstimatedSpeedCallback(msg);
    return;

}

void OdometryStateReceiver::printDroneGMREstimatedSpeedCallback(const droneMsgsROS::droneSpeeds::ConstPtr &msg)
{
    log(Info,std::string("Received dx from EstimatedSpeed_droneGMR_wrt_GFF: ")+ boost::lexical_cast<std::string>(msg->dx));
    log(Info,std::string("Received dy from EstimatedSpeed_droneGMR_wrt_GFF: ")+ boost::lexical_cast<std::string>(msg->dy));
    log(Info,std::string("Received dz from EstimatedSpeed_droneGMR_wrt_GFF:")+ boost::lexical_cast<std::string>(msg->dz));
    log(Info,std::string("Received dyaw from EstimatedSpeed_droneGMR_wrt_GFF: ")+ boost::lexical_cast<std::string>( msg->dyaw));
    log(Info,std::string("Received dpitch from EstimatedSpeed_droneGMR_wrt_GFF:  ")+ boost::lexical_cast<std::string>(msg->dpitch));
    log(Info,std::string("Received droll from EstimatedSpeed_droneGMR_wrt_GFF: ")+ boost::lexical_cast<std::string>(msg->droll));
    return;

}

void OdometryStateReceiver::droneSOEstimatedPoseCallback(const droneMsgsROS::dronePose::ConstPtr &msg)
{
    drone_pose_msgs=*msg;
    Q_EMIT parameterReceived();
    Q_EMIT updateStatus();
    //printDroneSOEstimatedPoseCallback(msg);
    return;

}

void OdometryStateReceiver::printDroneSOEstimatedPoseCallback(const droneMsgsROS::dronePose::ConstPtr &msg)
{
    log(Info,std::string("Received pitch from SOEstimatedPose: ")+ boost::lexical_cast<std::string>(msg->pitch));
    log(Info,std::string("Received roll from SOEstimatedPose: ")+ boost::lexical_cast<std::string>(msg->roll));
    log(Info,std::string("Received yaw from SOEstimatedPose:")+ boost::lexical_cast<std::string>(msg->yaw));
    log(Info,std::string("Received pos.x from SOEstimatedPose: ")+ boost::lexical_cast<std::string>(msg->x));
    log(Info,std::string("Received pos.y from SOEstimatedPose:")+ boost::lexical_cast<std::string>(msg->y));
    log(Info,std::string("Received pos.z from SOEstimatedPose: ")+ boost::lexical_cast<std::string>(msg->z));
    return;

}

void OdometryStateReceiver::droneSOEstimatedSpeedsCallback(const droneMsgsROS::droneSpeeds::ConstPtr &msg)
{
    drone_speeds_msgs=*msg;
    Q_EMIT parameterReceived();
    Q_EMIT updateStatus();
    //printDroneSOEstimatedSpeedsCallback(msg);
    return;

}

void OdometryStateReceiver::printDroneSOEstimatedSpeedsCallback(const droneMsgsROS::droneSpeeds::ConstPtr &msg)
{
    log(Info,std::string("Received dx from SOEstimatedSpeeds: ")+ boost::lexical_cast<std::string>(msg->dx));
    log(Info,std::string("Received dy from SOEstimatedSpeeds: ")+ boost::lexical_cast<std::string>(msg->dy));
    log(Info,std::string("Received dz from SOEstimatedSpeeds: ")+ boost::lexical_cast<std::string>(msg->dz));
    log(Info,std::string("Received dyaw from SOEstimatedSpeeds: ")+ boost::lexical_cast<std::string>(msg->dyaw));
    log(Info,std::string("Received dpitch from SOEstimatedSpeeds: ")+ boost::lexical_cast<std::string>(msg->dpitch));
    log(Info,std::string("Received droll from SOEstimatedSpeeds: ")+ boost::lexical_cast<std::string>(msg->droll));
    return;

}

void OdometryStateReceiver::dronePoseCallback(const droneMsgsROS::dronePose::ConstPtr &msg)
{
    drone_controller_pose_msgs=*msg;
    Q_EMIT parameterReceived();
    Q_EMIT updateStatus();
   // printDronePoseCallback(msg);

    return;
}

void OdometryStateReceiver::printDronePoseCallback(const droneMsgsROS::dronePose::ConstPtr &msg)
{
    log(Info,std::string("Received pitch from Controller:  ")+ boost::lexical_cast<std::string>(msg->pitch));
    log(Info,std::string("Received roll from Controller: ")+ boost::lexical_cast<std::string>(msg->roll));
    log(Info,std::string("Received yaw from Controller: ")+ boost::lexical_cast<std::string>(msg->yaw));
    log(Info,std::string("Received pos.x from Controller: ")+ boost::lexical_cast<std::string>(msg->x));
    log(Info,std::string("Received pos.y from Controller: ")+ boost::lexical_cast<std::string>(msg->y));
    log(Info,std::string("Received pos.z from Controller: ")+ boost::lexical_cast<std::string>( msg->z));
}

void OdometryStateReceiver::droneSpeedsCallback(const droneMsgsROS::droneSpeeds::ConstPtr &msg)
{
     drone_controller_speeds_msgs=*msg;
     Q_EMIT parameterReceived();
     Q_EMIT updateStatus();
     //printDroneSpeedsCallback(msg);
     return;
}
void OdometryStateReceiver::printDroneSpeedsCallback(const droneMsgsROS::droneSpeeds::ConstPtr &msg)
{
    log(Info,std::string("Received dx from Controller:  ")+ boost::lexical_cast<std::string>(msg->dx) );
    log(Info,std::string("Received dy from Controller: ")+ boost::lexical_cast<std::string>(msg->dy) );
    log(Info,std::string("Received dz from Controller: ")+ boost::lexical_cast<std::string>(msg->dz) );
    log(Info,std::string("Received dyaw from Controller: ")+ boost::lexical_cast<std::string>(msg->dyaw) );
    log(Info,std::string("Received dpitch from Controller: ")+ boost::lexical_cast<std::string>(msg->dpitch) );
    log(Info,std::string("Received droll from Controller: ")+ boost::lexical_cast<std::string>(msg->droll) );
}
void OdometryStateReceiver::droneArucoEyeObservationCallback(const droneMsgsROS::obsVector::ConstPtr &msg)
{

    // TODO
    return;

}

void OdometryStateReceiver::log( const LogLevel &level, const std::string &msg) {
	logging_model.insertRows(logging_model.rowCount(),1);
	std::stringstream logging_model_msg;
	switch ( level ) {
		case(Debug) : {
				ROS_DEBUG_STREAM(msg);
				logging_model_msg << "[DEBUG] [" << ros::Time::now() << "]: " << msg;
				break;
		}
		case(Info) : {
				ROS_INFO_STREAM(msg);
				logging_model_msg << "[INFO] [" << ros::Time::now() << "]: " << msg;
				break;
		}
		case(Warn) : {
				ROS_WARN_STREAM(msg);
				logging_model_msg << "[INFO] [" << ros::Time::now() << "]: " << msg;
				break;
		}
		case(Error) : {
				ROS_ERROR_STREAM(msg);
				logging_model_msg << "[ERROR] [" << ros::Time::now() << "]: " << msg;
				break;
		}
		case(Fatal) : {
				ROS_FATAL_STREAM(msg);
				logging_model_msg << "[FATAL] [" << ros::Time::now() << "]: " << msg;
				break;
		}
	}
	QVariant new_row(QString(logging_model_msg.str().c_str()));
	logging_model.setData(logging_model.index(logging_model.rowCount()-1),new_row);
	Q_EMIT loggingUpdated(); // used to readjust the scrollbar
}


