# Iot Device Gateway - IoT  Navigation System
A [rtos]() navigation system based on HTTP request. Developed with my computer engineering students at the [FIAP]() university - São Paulo, Brasil.

- [Setup](#local-setup)
- [Development](#development)
- [Contribution Guide](#contribution-guide)


## Local Setup

The hardware used id a robotic kit provided by [robocore](), and the sensor MPU6060:

![explorer-kit](https://www.robocore.net/upload/tutoriais/345_header_H.jpg)

The controller is an esp32 embedded on board VESPA developed by robocore for robotic projects, and the the follow diagram details the  pinout:

![vespa-conections](https://s3-sa-east-1.amazonaws.com/robocore-lojavirtual/1439/vespa_esquematica-01.jpg)

It is necesary to install the [esp-idf] on your computer, we recomend using the vscode plugin.

## Contribution Guide

You will need to create a new branch, follow this convention:

```bash
<turma>_<checkpoint>_<rm>
```
for example:

```bash
5ECR_Checkpoint2-A_rm3456
```

Create a Pull Request and add me to revise and aprove the features/changes.

## Checkpoint2-A (2 ptos) - due 11/09 00:00

At this time we have been created two task, at the collision task, we recibe the distance and print it like a decimal on meters only:

![distance-example](https://drive.google.com/file/d/1bdrUqq8ZABwk_B0XuDA11Y6nwHBAHkGe/view?usp=sharing)

we should storage meters and centimeters, please create a new structure to storage this distance values.




