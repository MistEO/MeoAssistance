import logging


def set_logger_level(level):
    logging.getLogger().setLevel(level)


def setup_logger(logger_name, log_file):
    logger = logging.getLogger(logger_name)

    console_handler = logging.StreamHandler()
    console_handler.setFormatter(logging.Formatter('%(levelname)s:%(message)s'))

    file_handler = logging.FileHandler(log_file)
    file_handler.setFormatter(logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s'))

    logger.addHandler(file_handler)
    logger.addHandler(console_handler)
    return logger


def get_logger(logger_name):
    setup_logger(logger_name, 'task_cli.log')
    return logging.getLogger(logger_name)
