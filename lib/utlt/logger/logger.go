package main

import (
	"C"
	"os"
	"time"

	formatter "github.com/antonfisher/nested-logrus-formatter"
	"github.com/sirupsen/logrus"

	logger_util "github.com/free5gc/util/logger"
)

var log *logrus.Logger
var UpfUtilLog *logrus.Entry

func init() {
	log = logrus.New()
	log.SetReportCaller(false)

	log.Formatter = &formatter.Formatter{
		TimestampFormat: time.RFC3339,
		TrimMessages:    true,
		NoFieldsSpace:   true,
		HideKeys:        true,
		FieldsOrder:     []string{"component", "category"},
	}

	UpfUtilLog = log.WithFields(logrus.Fields{"component": "UPF", "category": "Util"})
}

func logFileHook(logNfPath string, log5gcPath string) error {
	if fullPath, err := logger_util.CreateFree5gcLogFile(log5gcPath); err == nil {
		if fullPath != "" {
			free5gcLogHook, hookErr := logger_util.NewFileHook(fullPath, os.O_CREATE|os.O_APPEND|os.O_RDWR, 0o666)
			if hookErr != nil {
				return hookErr
			}
			log.Hooks.Add(free5gcLogHook)
		}
	} else {
		return err
	}

	if fullPath, err := logger_util.CreateNfLogFile(logNfPath, "upf.log"); err == nil {
		selfLogHook, hookErr := logger_util.NewFileHook(fullPath, os.O_CREATE|os.O_APPEND|os.O_RDWR, 0o666)
		if hookErr != nil {
			return hookErr
		}
		log.Hooks.Add(selfLogHook)
	} else {
		return err
	}

	return nil
}

func SetLogLevel(level logrus.Level) {
	UpfUtilLog.Infoln("Set log level:", level)
	log.SetLevel(level)
}

//export UpfUtilLog_FileHook
func UpfUtilLog_FileHook(logNfPath string, log5gcPath string) bool {
	if err := logFileHook(logNfPath, log5gcPath); err != nil {
		UpfUtilLog.Errorln("Error: log file hook: ", err)
		return false
	} else {
		return true
	}
}

//export UpfUtilLog_SetLogLevel
func UpfUtilLog_SetLogLevel(levelString string) bool {
	level, err := logrus.ParseLevel(levelString)
	if err == nil {
		SetLogLevel(level)
		return true
	} else {
		UpfUtilLog.Errorln("Error: invalid log level: ", levelString)
		return false
	}
}

//export UpfUtilLog_Panicln
func UpfUtilLog_Panicln(comment string) {
	UpfUtilLog.Panicln(comment)
}

//export UpfUtilLog_Fatalln
func UpfUtilLog_Fatalln(comment string) {
	UpfUtilLog.Fatalln(comment)
}

//export UpfUtilLog_Errorln
func UpfUtilLog_Errorln(comment string) {
	UpfUtilLog.Errorln(comment)
}

//export UpfUtilLog_Warningln
func UpfUtilLog_Warningln(comment string) {
	UpfUtilLog.Warningln(comment)
}

//export UpfUtilLog_Infoln
func UpfUtilLog_Infoln(comment string) {
	UpfUtilLog.Infoln(comment)
}

//export UpfUtilLog_Debugln
func UpfUtilLog_Debugln(comment string) {
	UpfUtilLog.Debugln(comment)
}

//export UpfUtilLog_Traceln
func UpfUtilLog_Traceln(comment string) {
	UpfUtilLog.Traceln(comment)
}

func main() {}
