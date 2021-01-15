package main

import (
	"C"
	"os"
	"time"

	formatter "github.com/antonfisher/nested-logrus-formatter"
	"github.com/sirupsen/logrus"

	"free5gc/lib/logger_conf"
	"free5gc/lib/logger_util"
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

	free5gcLogHook, err := logger_util.NewFileHook(logger_conf.Free5gcLogFile, os.O_CREATE|os.O_APPEND|os.O_RDWR, 0666)
	if err == nil {
		log.Hooks.Add(free5gcLogHook)
	}

	selfLogHook, err := logger_util.NewFileHook(logger_conf.NfLogDir+"upf.log", os.O_CREATE|os.O_APPEND|os.O_RDWR, 0666)
	if err == nil {
		log.Hooks.Add(selfLogHook)
	}

	UpfUtilLog = log.WithFields(logrus.Fields{"component": "UPF", "category": "Util"})
}

func SetLogLevel(level logrus.Level) {
	UpfUtilLog.Infoln("Set log level:", level)
	log.SetLevel(level)
}

//func SetReportCaller(reportCaller bool) {
//	UpfUtilLog.Infoln("Report caller:", reportCaller)
//	log.SetReportCaller(reportCaller)
//}

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
