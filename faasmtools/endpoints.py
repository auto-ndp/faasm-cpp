from configparser import ConfigParser
from os.path import expanduser, join, exists

FAASM_INI_FILE = join(expanduser("~"), ".config", "faasm.ini")

DEFAULT_KNATIVE_HEADERS = {"Host": "faasm-worker.faasm.example.com"}

DEFAULT_INVOKE_HOST = "worker"
DEFAULT_UPLOAD_HOST = "upload"
DEFAULT_INVOKE_PORT = 8080
DEFAULT_UPLOAD_PORT = 8002


def get_all_worker_addresses():
    faasm_config_exists_or_create()
    
    all_workers = get_faasm_ini_value("Faasm", "all_workers").split(",")
    return all_workers

def faasm_config_exists_or_create():
    if not exists(FAASM_INI_FILE):
        # Create the file if it doesn't exist
        ConfigParser().write(FAASM_INI_FILE)
        
        # Set Faasm upload and invoke hosts
        config = ConfigParser()
        config.read(FAASM_INI_FILE)
        config["Faasm"] = {
            "upload_host": DEFAULT_UPLOAD_HOST,
            "upload_port": DEFAULT_UPLOAD_PORT,
            "invoke_host": DEFAULT_INVOKE_HOST,
            "invoke_port": DEFAULT_INVOKE_PORT,
            "all_workers": "worker-0,worker-1,worker-2",
        }
        with open(FAASM_INI_FILE, "a") as fh:
            config.write(fh)


def get_faasm_ini_value(section, key):
    if not exists(FAASM_INI_FILE):
        print("Expected to find faasm config at {}".format(FAASM_INI_FILE))
        raise RuntimeError("Did not find faasm config")

    config = ConfigParser()
    config.read(FAASM_INI_FILE)
    return config[section].get(key, "")


def get_faasm_upload_host_port():
    faasm_config_exists_or_create()

    host = get_faasm_ini_value("Faasm", "upload_host")
    port = get_faasm_ini_value("Faasm", "upload_port")

    return host, port


def get_faasm_invoke_host_port():
    faasm_config_exists_or_create()
    
    host = get_faasm_ini_value("Faasm", "invoke_host")
    port = get_faasm_ini_value("Faasm", "invoke_port")

    return host, port


def get_knative_headers():
    faasm_config_exists_or_create()
    
    knative_host = get_faasm_ini_value("Faasm", "knative_host")

    headers = {"Host": knative_host}

    return headers
