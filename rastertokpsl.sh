#!/usr/bin/env bash

export DEVICE_URI=""
export PPD=/private/etc/cups/ppd/fs_1025mfp.ppd

KPSL="${1%.*}.kpsl"
echo "RAS=${1} > KPSL=${KPSL}"

/Library/Printers/Kyocera/kpsl/rastertokpsl.app/Contents/MacOS/rastertokpsl 804 svolkov "title_1" 1 "AP_ColorMatchingMode=AP_ApplicationColorMatching AP_D_InputSlot= nocollate com.apple.print.DocumentTicket.PMSpoolFormat=application/pdf com.apple.print.JobInfo.PMApplicationName=Word com.apple.print.JobInfo.PMJobName=Microsoft\ Word\ -\ 20150928\ Предложение\ МТС\ интернет\ спортзал.doc com.apple.print.JobInfo.PMJobOwner=svolkov com.apple.print.PageToPaperMappingMediaName=A4 com.apple.print.PageToPaperMappingType..n.=1 com.apple.print.PrinterInfo.PMColorDeviceID..n.=4634 com.apple.print.PrintSettings.PMColorSpaceModel..n.=1 com.apple.print.PrintSettings.PMCopies..n.=1 com.apple.print.PrintSettings.PMCopyCollate..b. com.apple.print.PrintSettings.PMDestinationType..n.=1 com.apple.print.PrintSettings.PMFirstPage..n.=1 com.apple.print.PrintSettings.PMLastPage..n.=2147483647 com.apple.print.PrintSettings.PMPageRange..a.0..n.=1 com.apple.print.PrintSettings.PMPageRange..a.1..n.=2147483647 DestinationPrinterID=fs_1025mfp media=A4 PaperInfoIsSuggested..b. pserrorhandler-requested=standard job-uuid=urn:uuid:31ffe93d-f5a1-35ae-4d89-10558ee07d4d job-originating-host-name=localhost time-at-creation=1443812222 time-at-processing=1443812222 job-impressions=1 com.apple.print.PrintSettings.PMTotalSidesImaged..n.=1 sides=one-sided Duplex=None com.apple.print.PrintSettings.PMTotalBeginPages..n.=1 PageSize=A4" ${1} > ${KPSL}