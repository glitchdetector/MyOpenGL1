﻿#pragma once

struct Level
{
    std::vector<Entity> entities;
    Transformation playerStart;
    Transformation cameraPosition;
    std::string levelName;

    Level(std::string levelFile)
    {
        std::ifstream in;
        in.open(levelFile);

        std::getline(in, levelName);
        std::cout << levelName << std::endl;

        in
            >> playerStart.x
            >> playerStart.y
            >> playerStart.z;

        std::cout << "Player start " << playerStart.x << ", " << playerStart.y << ", " << playerStart.z << std::endl;

        in
            >> cameraPosition.x
            >> cameraPosition.y
            >> cameraPosition.z
			>> cameraPosition.pitch
            >> cameraPosition.yaw
            >> cameraPosition.roll;

        std::cout << "Camera location " << cameraPosition.x << ", " << cameraPosition.y << ", " << cameraPosition.z << std::endl;
        std::cout << "Camera rotation " << cameraPosition.pitch << ", " << cameraPosition.yaw << ", " << cameraPosition.roll << std::endl;

        int nEntities;
        in
    		>> nEntities;

        for (int i = 0; i < nEntities; i++)
        {
            Entity entity;
            std::string fileName;
            in
        		>> fileName;

            in
                >> entity.transformation.x
                >> entity.transformation.y
                >> entity.transformation.z
                >> entity.transformation.pitch
                >> entity.transformation.yaw
                >> entity.transformation.roll;

            in
                >> entity.collision.x_relative
                >> entity.collision.y_relative
                >> entity.collision.z_relative
                >> entity.collision.x_size
                >> entity.collision.y_size
                >> entity.collision.z_size;

            ObjectFileReturnInfo objectLoadReturn = ReadObjectFile(fileName, entity.vertices, entity.indices);
            objectLoadReturn.print();

            entities.push_back(entity);
        }
    }
};