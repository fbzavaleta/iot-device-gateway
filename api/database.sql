CREATE TABLE trajectory (
                id  INT NOT NULL AUTO_INCREMENT,
                angle INT NOT NULL,
                translation VARCHAR(250) NOT NULL,                                     
                PRIMARY KEY (id)
);

CREATE TABLE device (
                id  INT NOT NULL AUTO_INCREMENT,
                mpu6050 BOOL NOT NULL,
                distance_sensor BOOL NOT NULL,
                reflex_sensor BOOL NOT NULL,
                trajectory_id  INT,                                    
                PRIMARY KEY (id),
                FOREIGN KEY (trajectory_id) REFERENCES trajectory(id)
);

CREATE TABLE navigation (
                id  INT NOT NULL AUTO_INCREMENT,
                device_id INT NOT NULL,
                acce_x FLOAT NOT NULL,
                acce_y FLOAT NOT NULL,
                acce_z FLOAT NOT NULL,    
                obstacles FLOAT NOT NULL,                                                                
                PRIMARY KEY (id),
                FOREIGN KEY (device_id) REFERENCES device(id)                
);