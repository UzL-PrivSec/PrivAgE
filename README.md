# Distributed Secure DP Pipeline 

## Description
A software suit for the secure summation protocol, which involves a server as well as different clients.
This project aims to help in the creation of software which wants to build upon the secure summation protocol.
The server is written in Python using django for easy extensibility with your own modules. 
Meanwhile this pipeline also provides the client code in kotlin, making it easy to integrate into android applications.
However the clients currently lack graphical interfaces in favour of scalability during the development of your projects.

## Installation and Deployment
We provide two different Dockerfiles as a method for fast and simple deployment.
Those are located at `client/simulation/Dockerfile` and `server/Dockerfile` respectively.
This section will walk you through the process of setting up the server as well as the simulated clients, in their designated Docker-containers.

### Step 1: Installing Docker.
First we need to install Docker as our container-engine.
You can freely choose if you want to use Docker Desktop or the CLI for this, but in this guide we will showcase the process using the CLI version.
Also we will assume that you use Ubuntu, since it is often used for server environments.
In case you are using something else, please consider the Docker documentation and continue with step 2 afterwards: https://docs.docker.com/engine/install/

Before we start make sure that you have one of the following version installed:
- Ubuntu Noble 24.04 (LTS)
- Ubuntu Jammy 22.04 (LTS)
- Ubuntu Focal 20.04 (LTS)

Next up you might need to uninstall old versions of Docker packages, since some maintainers of those Ubuntu versions provide unofficial Docker distributions which will conflict with the official docker version. 
We can do this easily for all packages (at the time of writing) with the following code snippet:
`for pkg in docker.io docker-doc docker-compose docker-compose-v2 podman-docker containerd runc; do sudo apt-get remove $pkg; done`
You can also find this snippet as well as a detailed description in the docker documentation under the section for Ubuntu.

For this next part you need to follow the guide on https://docs.docker.com/engine/install/ubuntu/#install-using-the-repository until the verification at the very end.
These steps aren't provided here, since the location of the keyring or the repository might change someday, thus rendering a potential code snippet here unusable.
Also if docker does not run immediately, the service might not have been started yet.
You can check this by using `sudo systemctl status docker`.
Should the daemon be inactive, you can easily activate it with this command: `sudo systemctl start docker`.
If this does not fix the issue, try restarting your machine and type in the command again.

### Step 2: Downloading our base image.
Since we now have docker up and running we need to download our base image from the Docker official Images Hub.
Both of our Dockerfiles use the same base image, which also is Ubuntu.
You can download it using the terminal by typing the following command: `docker pull ubuntu`

### Step 3: Building the images from the Dockerfiles
The Dockerfiles can be used as they are provided without changing anything in them.
As a reminder, you can find them in `client/simulation/Dockerfile` and `server/Dockerfile`. We will be building them without changing the directory, that means from the root of our repository.
The command is essentially the same for both with minimal changes for more readability when working with the containers later on. If you work with more containers than just our project, feel free to change the tags (client, server) as you see fit.
- `docker build -t client client/simulation/`
- `docker build -t server server/`

**IMPORTANT NOTE:** The building process might take a while and can on some systems include some warnings, which might be displayed in red. Do not worry those do not mean that anything is broken. The software should still work as intended as long as the build process does complete all specified steps.

### Step 4: Deployment of the containers.
To be done.
