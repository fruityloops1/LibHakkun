import sys
import os
import shutil
import ftplib
import subprocess

build_dir = sys.argv[1]
project_name = sys.argv[2]
title_id = int(sys.argv[3].removeprefix('0x'), 16)
module_binary = sys.argv[4]
has_rtld = sys.argv[5] == 'TRUE'
is_standalone = len(sys.argv) == 7 and sys.argv[6] == 'TRUE'
layeredfs_dir = f"atmosphere/contents/{title_id:016X}"
exefs_dir = f"{layeredfs_dir}/exefs"
sd_exefs_dir = f"{build_dir}/sd/{exefs_dir}"
sd_layeredfs_dir = f'{build_dir}/sd/{layeredfs_dir}'

def deploy_sd():
    print("-- Deploying to SD folder")
    try:
        shutil.rmtree(sd_exefs_dir)
    except FileNotFoundError:
        pass
    os.makedirs(sd_exefs_dir, exist_ok=True)

    if not is_standalone:
        shutil.copyfile(f"{build_dir}/main.npdm", f"{sd_exefs_dir}/main.npdm")
        shutil.copyfile(f"{build_dir}/{project_name}.nso", f"{sd_exefs_dir}/{module_binary}")
        if has_rtld:
            shutil.copyfile(f"{build_dir}/rtld.nso", f"{sd_exefs_dir}/rtld")
    else:
        shutil.copyfile(f"{build_dir}/exefs.nsp", f"{sd_layeredfs_dir}/exefs.nsp")

def deploy_ftp():
    ftp_ip = os.environ['HAKKUN_FTP_IP']
    ftp_port = int(os.environ['HAKKUN_FTP_PORT']) if os.environ.get('HAKKUN_FTP_PORT') is not None else 5000
    ftp_user = os.environ.get('HAKKUN_FTP_USER')
    ftp_password = os.environ.get('HAKKUN_FTP_PASS')
    if (ftp_password is None):
        ftp_password = os.environ.get('HAKKUN_FTP_PASSWORD')
    print(f'-- Deploying to ftp://{ftp_ip}:{ftp_port}')

    ftp = ftplib.FTP()
    try:
        ftp.connect(ftp_ip, ftp_port)
    except OSError:
        print('-- Could not connect to FTP host')
        sys.exit(1)

    if (ftp_user is not None):
        ftp.login(ftp_user, ftp_password)

    def upload(file, path):
        print(f'-- Uploading {file} to ftp://{ftp_ip}:{ftp_port}/{path}')
        with open(file, 'rb') as io:
            try:
                ftp.storbinary(f'STOR {path}', io)
            except ftplib.error_temp as e:
                print(f'-- Could not upload file: {str(e).split(None, 1)}')
                sys.exit(1)

    upload(f"{build_dir}/main.npdm", f"{exefs_dir}/main.npdm")
    upload(f"{build_dir}/{project_name}.nso", f"{exefs_dir}/{module_binary}")
    if has_rtld:
        upload(f"{build_dir}/rtld.nso", f"{exefs_dir}/rtld")
    if is_standalone:
        upload(f"{build_dir}/exefs.nsp", f"{layeredfs_dir}/exefs.nsp")

# build/exefs
shutil.copyfile(f"{build_dir}/main.npdm", f"{build_dir}/exefs/main.npdm")
shutil.copyfile(f"{build_dir}/{project_name}.nso", f"{build_dir}/exefs/{module_binary}")
if has_rtld:
    shutil.copyfile(f"{build_dir}/rtld.nso", f"{build_dir}/exefs/rtld")

# build/exefs.nsp
if is_standalone:
    print('-- Building exefs.nsp')
    subprocess.check_output(['python', f'{os.path.dirname(os.path.abspath(__file__))}/pfs0/build_pfs0.py', f"{build_dir}/exefs/", f"{build_dir}/exefs.nsp"])

deploy_sd()
if (os.environ.get('HAKKUN_FTP_IP') is not None):
    deploy_ftp()
