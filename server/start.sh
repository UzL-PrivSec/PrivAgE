#!/bin/bash

. server_env/bin/activate

python3 manage.py flush --no-input && python3 manage.py runserver 0.0.0.0:8888

deactivate
