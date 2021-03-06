/******************************************************************************
 * Copyright (c) 2011
 * Locomotec
 *
 * Author:
 * Sebastian Blumenthal
 *
 *
 * This software is published under a dual-license: GNU Lesser General Public
 * License LGPL 2.1 and BSD license. The dual-license implies that users of this
 * code may choose which terms they prefer.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * * Neither the name of Locomotec nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License LGPL as
 * published by the Free Software Foundation, either version 2.1 of the
 * License, or (at your option) any later version or the BSD license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License LGPL and the BSD license for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License LGPL and BSD license along with this program.
 *
 ******************************************************************************/

#include "msg_translation/MsgTranslationWrapper.h"
//#include "node/joint_state_observer_oodl.h"

//#include <youbot_trajectory_action_server/joint_trajectory_action.h>
#include <sstream>

namespace node
{

MsgTranslationWrapper::MsgTranslationWrapper()
{
}


MsgTranslationWrapper::MsgTranslationWrapper(ros::NodeHandle n) :
node(n)
{

    //nodeConfiguration.hasBase = false;
    //nodeConfiguration.hasArms = false;
   
   // youBotChildFrameID = "base_link"; //holds true for both: base and arm
     jointTrajectoryMessages.clear();
     gripperTrajectoryMessages.clear();

    
    //gripperCycleCounter = 0;
   // diagnosticNameArm = "platform_Arm";
    //diagnosticNameBase = "platform_Base";
   
}

MsgTranslationWrapper::~MsgTranslationWrapper()
{
    this->stop();
   
}


void MsgTranslationWrapper::initialize()
{
    int armIndex;
   // youbot::JointName jointNameParameter;
    std::string jointName;
    stringstream topicName;
   // stringstream serviceName;

          
        NodeArmConfiguration tmpArmConfig;
        nodeConfiguration.nodeArmConfigurations.push_back(tmpArmConfig);

        armIndex = static_cast<int> (nodeConfiguration.nodeArmConfigurations.size()) - 1;
       
       // nodeConfiguration.nodeArmConfigurations[armIndex].armID = armName;

        topicName.str("");
        topicName << "arm_" << (armIndex + 1) << "/";
        nodeConfiguration.nodeArmConfigurations[armIndex].commandTopicName = topicName.str(); // e.g. arm_1/


       topicName.str("");
       topicName << nodeConfiguration.nodeArmConfigurations[armIndex].commandTopicName << "arm_controller/position_command"; // e.g. arm_1/arm_controller/positionCommand
       nodeConfiguration.nodeArmConfigurations[armIndex].armPositionCommandSubscriber = node.subscribe<brics_actuator::JointPositions> (topicName.str(), 1000, boost::bind(&MsgTranslationWrapper::armPositionsCommandCallback, this, _1, armIndex));


        topicName.str("");
        topicName << nodeConfiguration.nodeArmConfigurations[armIndex].commandTopicName << "arm_controller/velocity_command";
         nodeConfiguration.nodeArmConfigurations[armIndex].armVelocityCommandSubscriber = node.subscribe<brics_actuator::JointVelocities > (topicName.str(), 1000, boost::bind(&MsgTranslationWrapper::armVelocitiesCommandCallback, this, _1, armIndex));


       topicName.str("");
       topicName << nodeConfiguration.nodeArmConfigurations[armIndex].commandTopicName << "arm_controller/torques_command";
       nodeConfiguration.nodeArmConfigurations[armIndex].armTorquesCommandSubscriber = node.subscribe<brics_actuator::JointTorques > (topicName.str(), 1000, boost::bind(&MsgTranslationWrapper::armTorquesCommandCallback, this, _1, armIndex));


       topicName.str("");
       topicName << nodeConfiguration.nodeArmConfigurations[armIndex].commandTopicName << "arm_controller/command";
       nodeConfiguration.nodeArmConfigurations[armIndex].armCommandPublisher = node.advertise<trajectory_msgs::JointTrajectory> (topicName.str(), 1);




        topicName.str("");
        topicName << nodeConfiguration.nodeArmConfigurations[armIndex].commandTopicName << "gripper_controller/position_command";
        nodeConfiguration.nodeArmConfigurations[armIndex].gripperPositionCommandSubscriber = node.subscribe<brics_actuator::JointPositions > (topicName.str(), 1000, boost::bind(&MsgTranslationWrapper::gripperPositionsCommandCallback, this, _1, armIndex));
       // nodetConfiguration.nodeArmConfigurations[armIndex].lastGripperCommand = 0.0;


       topicName.str("");
       topicName << nodeConfiguration.nodeArmConfigurations[armIndex].commandTopicName << "gripper_controller/command";
       nodeConfiguration.nodeArmConfigurations[armIndex].gripperCommandPublisher = node.advertise<trajectory_msgs::JointTrajectory> (topicName.str(), 1);



      trajectory_msgs::JointTrajectory dummyMessage;
      jointTrajectoryMessages.push_back(dummyMessage);

      trajectory_msgs::JointTrajectory dummyGripperMessage;
      gripperTrajectoryMessages.push_back(dummyGripperMessage);
   

        
        
}

void MsgTranslationWrapper::stop()
{

    
    for (int armIndex = 0; armIndex < static_cast<int> (nodeConfiguration.nodeArmConfigurations.size()); armIndex++) //delete each arm
    {
        

        nodeConfiguration.nodeArmConfigurations[armIndex].armCommandPublisher.shutdown();
        
        nodeConfiguration.nodeArmConfigurations[armIndex].armPositionCommandSubscriber.shutdown();
        nodeConfiguration.nodeArmConfigurations[armIndex].armVelocityCommandSubscriber.shutdown();
        nodeConfiguration.nodeArmConfigurations[armIndex].armTorquesCommandSubscriber.shutdown();
         // nodeConfiguration.nodeArmConfigurations[armIndex].armCommandPublisher.shutdown();
        nodeConfiguration.nodeArmConfigurations[armIndex].gripperPositionCommandSubscriber.shutdown();
        
    }

   // youBotConfiguration.hasArms = false;
    
    nodeConfiguration.nodeArmConfigurations.clear();
    jointTrajectoryMessages.clear();
    gripperTrajectoryMessages.clear();

    
}


void MsgTranslationWrapper::armPositionsCommandCallback(const brics_actuator::JointPositionsConstPtr& youbotArmCommand, int armIndex)
{
       ROS_DEBUG("Command for arm%i received", armIndex + 1);
       ROS_ASSERT(0 <= armIndex && armIndex < static_cast<int> (nodeConfiguration.nodeArmConfigurations.size()));


       std::vector <brics_actuator::JointValue> positions = youbotArmCommand->positions;

       currentPos.resize(5);
  
        if (youbotArmCommand->positions.size() < 1)
        {
            ROS_WARN("youBot driver received an invalid joint positions command.");
            return;
        }
       
        /* populate mapping between joint names and values  */
        std::map<string, double> jointNameToValueMapping;
        for (int i = 0; i < static_cast<int> (youbotArmCommand->positions.size()); ++i)
        {
                jointNameToValueMapping.insert(make_pair(youbotArmCommand->positions[i].joint_uri,youbotArmCommand->positions[i].value));   
        }

        ROS_ASSERT(nodeConfiguration.nodeArmConfigurations[armIndex].jointNames.size() == static_cast<unsigned int> (ArmDoF));
        
        for (int i = 0; i < ArmDoF; ++i)
        {
            /* check what is in map */
            map<string, double>::const_iterator jointIterator = jointNameToValueMapping.find(nodeConfiguration.nodeArmConfigurations[armIndex].jointNames[i]);
            
               if (jointIterator != jointNameToValueMapping.end())
               {
                /* set the joint value */
               
                   try{
                       currentPos[i] = positions[i].value;
                      
                       }
                   catch (std::exception& e)
                   {
                       std::string errorMessage = e.what();
                       ROS_WARN("Cannot set arm joint %i: %s", i + 1, errorMessage.c_str());
                    } 		 
                }
        
         }
   

}


void MsgTranslationWrapper::armVelocitiesCommandCallback(const brics_actuator::JointVelocitiesConstPtr& youbotArmCommand, int armIndex)
{
       ROS_DEBUG("Command for arm%i received", armIndex + 1);
       ROS_ASSERT(0 <= armIndex && armIndex < static_cast<int> (nodeConfiguration.nodeArmConfigurations.size()));

  
       std::vector <brics_actuator::JointValue> velocities = youbotArmCommand->velocities;
       currentVelo.resize(5);

        if (youbotArmCommand->velocities.size() < 1)
        {
            ROS_WARN("youBot driver received an invalid joint velocities command.");
            return;
        }


        /* populate mapping between joint names and values  */
        std::map<string, double> jointNameToValueMapping;
        for (int i = 0; i < static_cast<int> (youbotArmCommand->velocities.size()); ++i)
        {
                jointNameToValueMapping.insert(make_pair(youbotArmCommand->velocities[i].joint_uri, youbotArmCommand->velocities[i].value));
        }

        /* loop over all youBot arm joints and check if something is in the received message that requires action */
        ROS_ASSERT(nodeConfiguration.nodeArmConfigurations[armIndex].jointNames.size() == static_cast<unsigned int> (ArmDoF));

        for (int i = 0; i < ArmDoF; ++i)
        {

            /* check what is in map */
            map<string, double>::const_iterator jointIterator = jointNameToValueMapping.find(nodeConfiguration.nodeArmConfigurations[armIndex].jointNames[i]);
            if (jointIterator != jointNameToValueMapping.end())
            {

                /* set the desired joint value */
                try
                {
                    currentVelo[i] = velocities[i].value;
                       
                }
                catch (std::exception& e)
                {
                    std::string errorMessage = e.what();
                    ROS_WARN("Cannot set arm joint %i: %s", i + 1, errorMessage.c_str());
                }
             }
          }
       
}



void MsgTranslationWrapper::armTorquesCommandCallback(const brics_actuator::JointTorquesConstPtr& youbotArmCommand, int armIndex)
{
      ROS_DEBUG("Command for arm%i received", armIndex + 1);
      ROS_ASSERT(0 <= armIndex && armIndex < static_cast<int> (nodeConfiguration.nodeArmConfigurations.size()));

      std::vector <brics_actuator::JointValue> torques = youbotArmCommand->torques;

       currentTorq.resize(5);
         
          if (youbotArmCommand->torques.size() < 1)
          {
              ROS_WARN("youBot driver received an invalid joint torques command.");
              return;
          }
       
          /* populate mapping between joint names and values  */
          std::map<string, double> jointNameToValueMapping;
          for (int i = 0; i < static_cast<int> (youbotArmCommand->torques.size()); ++i)
          {
                  jointNameToValueMapping.insert(make_pair(youbotArmCommand->torques[i].joint_uri, youbotArmCommand->torques[i].value));
             
          }

          /* loop over all youBot arm joints and check if something is in the received message that requires action */
          ROS_ASSERT(nodeConfiguration.nodeArmConfigurations[armIndex].jointNames.size() == static_cast<unsigned int> (ArmDoF));

          for (int i = 0; i < ArmDoF; ++i)
          {

              /* check what is in map */
              map<string, double>::const_iterator jointIterator = jointNameToValueMapping.find(nodeConfiguration.nodeArmConfigurations[armIndex].jointNames[i]);
              if (jointIterator != jointNameToValueMapping.end())
              {

                  /* set the desired joint value */
                  
                  try
                  {
                      currentTorq[i] = torques[i].value;
                  }
                  catch (std::exception& e)
                  {
                      std::string errorMessage = e.what();
                      ROS_WARN("Cannot set arm joint %i: %s", i + 1, errorMessage.c_str());
                  }
              }
          }
          
}








void MsgTranslationWrapper::gripperPositionsCommandCallback(const brics_actuator::JointPositionsConstPtr& youbotGripperCommand, int armIndex)
{
	ROS_DEBUG("Command for gripper%i received", armIndex + 1);
	ROS_ASSERT(0 <= armIndex && armIndex < static_cast<int>(nodeConfiguration.nodeArmConfigurations.size()));


       std::vector <brics_actuator::JointValue> gripper = youbotGripperCommand->positions;

       currentGripperPos.resize(2);
	

		if (youbotGripperCommand->positions.size() < 1){
			ROS_WARN("youBot driver received an invalid gripper positions command.");
			return;
		}

		map<string, double>::const_iterator gripperIterator;
		
		/* populate mapping between joint names and values */
		std::map<string, double> jointNameToValueMapping;
		for (int i = 0; i < static_cast<int>(youbotGripperCommand->positions.size()); ++i) {
			
				jointNameToValueMapping.insert(make_pair(youbotGripperCommand->positions[i].joint_uri, youbotGripperCommand->positions[i].value));
			
		}

		
		/* check if something is in the received message that requires action for the left finger gripper */
		gripperIterator = jointNameToValueMapping.find(nodeConfiguration.nodeArmConfigurations[armIndex].gripperFingerNames[NodeArmConfiguration::LEFT_FINGER_INDEX]);
		if (gripperIterator != jointNameToValueMapping.end()) {
			ROS_DEBUG("Trying to set the left gripper finger to new value %f", gripperIterator->second);
			try {
			
                              currentGripperPos[0] = gripper[0].value;

			} catch (std::exception& e) {
				std::string errorMessage = e.what();
				ROS_WARN("Cannot set the left gripper finger: %s", errorMessage.c_str());
			}
		}

		/* check if something is in the received message that requires action for the right finger gripper */
		gripperIterator = jointNameToValueMapping.find(nodeConfiguration.nodeArmConfigurations[armIndex].gripperFingerNames[NodeArmConfiguration::RIGHT_FINGER_INDEX]);
		if (gripperIterator != jointNameToValueMapping.end()) {
			
			try {
				
                               currentGripperPos[1] = gripper[1].value;
                                

			} catch (std::exception& e) {
				std::string errorMessage = e.what();
				ROS_WARN("Cannot set the right gripper finger: %s", errorMessage.c_str());
			}
		}

		
}







void MsgTranslationWrapper::updateArmCommand()
{
        currentTime = ros::Time::now()  + ros::Duration(0.1);
            
        
       for (int armIndex = 0; armIndex < static_cast<int> (nodeConfiguration.nodeArmConfigurations.size()); armIndex++)
       {
           ROS_ASSERT(nodeConfiguration.nodeArmConfigurations.size() == jointTrajectoryMessages.size());
           //ROS_ASSERT(nodeConfiguration.nodeArmConfigurations.size() == gripperTrajectoryMessages.size());

          // currentTime = ros::Time::now()  + ros::Duration(0.1);
           jointTrajectoryMessages[armIndex].joint_names.resize(ArmDoF);

           jointTrajectoryMessages[armIndex].joint_names[0] = "arm_joint_1";
           jointTrajectoryMessages[armIndex].joint_names[1] = "arm_joint_2";
           jointTrajectoryMessages[armIndex].joint_names[2] = "arm_joint_3";
           jointTrajectoryMessages[armIndex].joint_names[3] = "arm_joint_4";
           jointTrajectoryMessages[armIndex].joint_names[4] = "arm_joint_5";


             trajectory_msgs::JointTrajectoryPoint  point ;
             point.positions.resize(ArmDoF);
             point.velocities.resize(ArmDoF);
             point.effort.resize(ArmDoF);        
            if(static_cast<int>(currentPos.size()) == ArmDoF)
              {           
                     for (int i = 0; i < ArmDoF; ++i)
                     {
                        point.positions[i] = currentPos[i] ;
                       
                       }                   
                }else{
                     
//0.05004093943948895, 0.1294137360159624, -0.21982068505249774, 0.15760491658190023, 0.0025035536784629997, 
//-0.03175738364753859, 0.019040908525729466, -0.043930074695166674, 0.024567172349286404, 0.060817421149235565, 
//-0.034732168550673986, 0.43880198765294803, -1.7113257736207594, 0.07196810529050797, 0.9989749747593785,      
                        point.positions[0] =  0.5004093943948895;
                        point.positions[1] =  0.1294137360159624;
                        point.positions[2] = -0.21982068505249774;
                        point.positions[3] =  0.15760491658190023;
                        point.positions[4] =  0.0025035536784629997;
               }
             if(static_cast<int>(currentVelo.size()) == ArmDoF)
              {  
                     for (int i = 0; i < ArmDoF; ++i)
                     {
                        point.velocities[i] = currentVelo[i];
                       
                       }
                }else{
                        point.velocities[0] = 1.03175738364753859;
                        point.velocities[1] = 1.019040908525729466;
                        point.velocities[2] = -1.043930074695166674;
                        point.velocities[3] = 1.024567172349286404;
                        point.velocities[4] = 1.060817421149235565;
               }
              if(static_cast<int>(currentTorq.size()) == ArmDoF)
              {  
                     for (int i = 0; i < ArmDoF; ++i)
                     {
                        point.effort[i] = currentTorq[i] ;
                       
                       }       
                }else{

                point.effort[0] = -0.034732168550673986;
                point.effort[1] = 0.43880198765294803;
                point.effort[2] = -1.7113257736207594;
                point.effort[3] = 0.07196810529050797;
                point.effort[4] = 0.9989749747593785;  
               }

                point.time_from_start = ros::Duration(0.1);
               jointTrajectoryMessages[armIndex].points.resize(1);   
               jointTrajectoryMessages[armIndex].points[0] = point;//.push_back(point);
               jointTrajectoryMessages[armIndex].header.stamp = currentTime;
             
  }


}





void MsgTranslationWrapper::updateGripperCommand()
{
/*
             * NOTE: gripper slide rails are always symmetric, but the fingers can be screwed in different
             * positions! The published values account for the distance between the gripper slide rails, not the fingers
             * themselves. Of course if the finger are screwed to the most inner position (i.e. the can close completely),
             * than it is correct.
             */
for (int armIndex = 0; armIndex < static_cast<int> (nodeConfiguration.nodeArmConfigurations.size()); armIndex++)
       {

           ROS_ASSERT(nodeConfiguration.nodeArmConfigurations.size() == gripperTrajectoryMessages.size());  // gripperTrajectoryMessages
           
           gripperTrajectoryMessages[armIndex].joint_names.resize(NumberOfFingers);

           gripperTrajectoryMessages[armIndex].joint_names[0] = "gripper_finger_joint_l";
           gripperTrajectoryMessages[armIndex].joint_names[1] = "gripper_finger_joint_r";
          

             trajectory_msgs::JointTrajectoryPoint  pointGripper ;

             pointGripper.positions.resize(NumberOfFingers);
             pointGripper.velocities.resize(NumberOfFingers);
             pointGripper.effort.resize(NumberOfFingers);
       
            if(static_cast<int>(currentGripperPos.size()) == NumberOfFingers)
              {           
                     for (int i = 0; i < NumberOfFingers; ++i)
                     {
                        pointGripper.positions[i] = currentGripperPos[i] ;
                       
                       }                   
                }else{                       
                        pointGripper.positions[0] =  0.05004093943948895;
                        pointGripper.positions[1] =  0.01294137360159624;                   
               }
             
                        pointGripper.velocities[0] = -0.03175738364753859;
                        pointGripper.velocities[1] = 1.019040908525729466;
                        
              
                        pointGripper.effort[0] = -0.034732168550673986;
                        pointGripper.effort[1] = 0.43880198765294803;
                        

                pointGripper.time_from_start = ros::Duration(0.1);

                gripperTrajectoryMessages[armIndex].points.resize(1);   
                gripperTrajectoryMessages[armIndex].points[0] = pointGripper;//.push_back(point);
                gripperTrajectoryMessages[armIndex].header.stamp = currentTime;  


  }
    
}




void MsgTranslationWrapper::publishOODLSensorReadings()
{
      
  
        for (int armIndex = 0; armIndex < static_cast<int> (nodeConfiguration.nodeArmConfigurations.size()); armIndex++)
        {

            nodeConfiguration.nodeArmConfigurations[armIndex].armCommandPublisher.publish(jointTrajectoryMessages[armIndex] );

            nodeConfiguration.nodeArmConfigurations[armIndex].gripperCommandPublisher.publish(gripperTrajectoryMessages[armIndex] );

        }



}



} // namespace node

/* EOF */

