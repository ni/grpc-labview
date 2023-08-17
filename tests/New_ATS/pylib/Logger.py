import logging
import datetime
import os
import random
class LoggerImpl:
    def __init__(self):
        self.logger = logging.getLogger('SignatureChecker')
        self.logger.setLevel(logging.DEBUG)
		# create a file handler which logs even DEBUG messages
        pyDir = os.path.dirname(os.path.realpath(__file__))+ "/logs"
        try:
            os.mkdir(pyDir)
        except Exception:
            pass
        random.seed();
        self.logFileName = os.path.join(pyDir, 'gRPC_TestSuite_Log_%s.txt' %(datetime.datetime.now().strftime("%Y-%B-%d_%H-%M-%S_"+str(random.randint(10000, 99999)))))
        fh = logging.FileHandler(self.logFileName, mode='w')
        fh.setLevel(logging.DEBUG)
        # create console handler with a higher level logger
        ch = logging.StreamHandler()
        ch.setLevel(logging.INFO)
        # add the handlers to the loggers
        self.logger.addHandler(fh)
        self.logger.addHandler(ch)
    def GetLogger(self):
        return self.logger
    def GetLogFileName(self):
        return os.path.abspath(self.logFileName)
    
class Logger:
	_loggerImpl = LoggerImpl()
	def GetLogger(self):
		return self._loggerImpl.GetLogger()
	def GetLogFileName(self):
		return self._loggerImpl.GetLogFileName()
def GetLogger():
	return Logger().GetLogger()
def GetLogFileName():
	return Logger().GetLogFileName()