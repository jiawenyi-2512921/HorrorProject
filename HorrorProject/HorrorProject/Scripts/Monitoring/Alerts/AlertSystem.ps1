# AlertSystem.ps1 - 告警系统主程序
param(
    [Parameter(Mandatory=$true)]
    [string]$Type,

    [Parameter(Mandatory=$true)]
    [ValidateSet("Info", "Warning", "Critical")]
    [string]$Severity,

    [Parameter(Mandatory=$true)]
    [string]$Message,

    [hashtable]$Details = @{}
)

$ErrorActionPreference = "Stop"
$MonitoringRoot = Split-Path -Parent $PSScriptRoot
$AlertDir = Join-Path $MonitoringRoot "Data\Alerts"
$LogDir = Join-Path $MonitoringRoot "Logs"
$ConfigFile = Join-Path $MonitoringRoot "alert-config.json"

New-Item -ItemType Directory -Force -Path $AlertDir, $LogDir | Out-Null

class Alert {
    [datetime]$Timestamp
    [string]$Type
    [string]$Severity
    [string]$Message
    [hashtable]$Details
    [string]$Id
    [bool]$Acknowledged
}

function Initialize-AlertConfig {
    if (Test-Path $ConfigFile) {
        return Get-Content $ConfigFile | ConvertFrom-Json -AsHashtable
    }

    $config = @{
        Channels = @{
            Console = $true
            Email = $false
            Slack = $false
            File = $true
        }
        Email = @{
            SmtpServer = "smtp.gmail.com"
            Port = 587
            From = "alerts@horrorproject.com"
            To = @("dev-team@horrorproject.com")
            Username = ""
            Password = ""
        }
        Slack = @{
            WebhookUrl = ""
            Channel = "#alerts"
        }
        RateLimits = @{
            MaxAlertsPerHour = 50
            DuplicateWindow = 300
        }
        SeverityThresholds = @{
            Info = $true
            Warning = $true
            Critical = $true
        }
    }

    $config | ConvertTo-Json -Depth 10 | Set-Content $ConfigFile
    return $config
}

function New-Alert {
    param(
        [string]$Type,
        [string]$Severity,
        [string]$Message,
        [hashtable]$Details
    )

    $alert = [Alert]::new()
    $alert.Timestamp = Get-Date
    $alert.Type = $Type
    $alert.Severity = $Severity
    $alert.Message = $Message
    $alert.Details = $Details
    $alert.Id = [guid]::NewGuid().ToString()
    $alert.Acknowledged = $false

    return $alert
}

function Test-DuplicateAlert {
    param(
        [Alert]$Alert,
        [int]$WindowSeconds
    )

    $cutoff = (Get-Date).AddSeconds(-$WindowSeconds)
    $recentAlerts = Get-ChildItem $AlertDir -Filter "alert-*.json" |
        Where-Object { $_.LastWriteTime -gt $cutoff } |
        ForEach-Object { Get-Content $_.FullName | ConvertFrom-Json }

    foreach ($recent in $recentAlerts) {
        if ($recent.Type -eq $Alert.Type -and
            $recent.Severity -eq $Alert.Severity -and
            $recent.Message -eq $Alert.Message) {
            return $true
        }
    }

    return $false
}

function Test-RateLimit {
    param([int]$MaxPerHour)

    $cutoff = (Get-Date).AddHours(-1)
    $recentCount = @(Get-ChildItem $AlertDir -Filter "alert-*.json" |
        Where-Object { $_.LastWriteTime -gt $cutoff }).Count

    return $recentCount -lt $MaxPerHour
}

function Send-ConsoleAlert {
    param([Alert]$Alert)

    $color = switch ($Alert.Severity) {
        "Info" { "Cyan" }
        "Warning" { "Yellow" }
        "Critical" { "Red" }
    }

    Write-Host ""
    Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor $color
    Write-Host "║                    ALERT NOTIFICATION                      ║" -ForegroundColor $color
    Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor $color
    Write-Host "Time: $($Alert.Timestamp.ToString('yyyy-MM-dd HH:mm:ss'))" -ForegroundColor $color
    Write-Host "Type: $($Alert.Type)" -ForegroundColor $color
    Write-Host "Severity: $($Alert.Severity)" -ForegroundColor $color
    Write-Host "Message: $($Alert.Message)" -ForegroundColor $color

    if ($Alert.Details.Count -gt 0) {
        Write-Host "Details:" -ForegroundColor $color
        foreach ($key in $Alert.Details.Keys) {
            Write-Host "  $key: $($Alert.Details[$key])" -ForegroundColor $color
        }
    }
    Write-Host ""
}

function Send-EmailAlert {
    param(
        [Alert]$Alert,
        [hashtable]$EmailConfig
    )

    if (-not $EmailConfig.Username -or -not $EmailConfig.Password) {
        Write-Host "Email credentials not configured" -ForegroundColor Yellow
        return
    }

    try {
        $subject = "[$($Alert.Severity)] $($Alert.Type) Alert"
        $body = @"
Alert Notification

Time: $($Alert.Timestamp.ToString('yyyy-MM-dd HH:mm:ss'))
Type: $($Alert.Type)
Severity: $($Alert.Severity)
Message: $($Alert.Message)

Details:
$($Alert.Details | ConvertTo-Json -Depth 5)

Alert ID: $($Alert.Id)
"@

        $securePassword = ConvertTo-SecureString $EmailConfig.Password -AsPlainText -Force
        $credential = New-Object System.Management.Automation.PSCredential($EmailConfig.Username, $securePassword)

        Send-MailMessage `
            -SmtpServer $EmailConfig.SmtpServer `
            -Port $EmailConfig.Port `
            -UseSsl `
            -Credential $credential `
            -From $EmailConfig.From `
            -To $EmailConfig.To `
            -Subject $subject `
            -Body $body `
            -ErrorAction Stop

        Write-Host "Email alert sent successfully" -ForegroundColor Green

    } catch {
        Write-Host "Failed to send email alert: $_" -ForegroundColor Red
    }
}

function Send-SlackAlert {
    param(
        [Alert]$Alert,
        [hashtable]$SlackConfig
    )

    if (-not $SlackConfig.WebhookUrl) {
        Write-Host "Slack webhook not configured" -ForegroundColor Yellow
        return
    }

    try {
        $color = switch ($Alert.Severity) {
            "Info" { "#36a64f" }
            "Warning" { "#ff9900" }
            "Critical" { "#ff0000" }
        }

        $payload = @{
            channel = $SlackConfig.Channel
            username = "HorrorProject Monitor"
            icon_emoji = ":warning:"
            attachments = @(
                @{
                    color = $color
                    title = "[$($Alert.Severity)] $($Alert.Type) Alert"
                    text = $Alert.Message
                    fields = @(
                        @{
                            title = "Time"
                            value = $Alert.Timestamp.ToString('yyyy-MM-dd HH:mm:ss')
                            short = $true
                        }
                        @{
                            title = "Alert ID"
                            value = $Alert.Id
                            short = $true
                        }
                    )
                    footer = "HorrorProject Monitoring"
                    ts = [int][double]::Parse((Get-Date -UFormat %s))
                }
            )
        } | ConvertTo-Json -Depth 10

        Invoke-RestMethod -Uri $SlackConfig.WebhookUrl -Method Post -Body $payload -ContentType 'application/json' -ErrorAction Stop
        Write-Host "Slack alert sent successfully" -ForegroundColor Green

    } catch {
        Write-Host "Failed to send Slack alert: $_" -ForegroundColor Red
    }
}

function Save-Alert {
    param([Alert]$Alert)

    $timestamp = $Alert.Timestamp.ToString("yyyyMMdd-HHmmss")
    $alertFile = Join-Path $AlertDir "alert-$timestamp-$($Alert.Id).json"

    $Alert | ConvertTo-Json -Depth 10 | Set-Content $alertFile

    # 记录到日志
    $logFile = Join-Path $LogDir "alerts.log"
    $logEntry = "[$($Alert.Timestamp.ToString('yyyy-MM-dd HH:mm:ss'))] [$($Alert.Severity)] $($Alert.Type): $($Alert.Message)"
    Add-Content -Path $logFile -Value $logEntry
}

function Send-Alert {
    param([Alert]$Alert)

    $config = Initialize-AlertConfig

    # 检查严重性阈值
    if (-not $config.SeverityThresholds[$Alert.Severity]) {
        Write-Host "Alert severity $($Alert.Severity) is disabled" -ForegroundColor Gray
        return
    }

    # 检查速率限制
    if (-not (Test-RateLimit -MaxPerHour $config.RateLimits.MaxAlertsPerHour)) {
        Write-Host "Alert rate limit exceeded" -ForegroundColor Yellow
        return
    }

    # 检查重复告警
    if (Test-DuplicateAlert -Alert $Alert -WindowSeconds $config.RateLimits.DuplicateWindow) {
        Write-Host "Duplicate alert suppressed" -ForegroundColor Gray
        return
    }

    # 保存告警
    Save-Alert -Alert $Alert

    # 发送到各个渠道
    if ($config.Channels.Console) {
        Send-ConsoleAlert -Alert $Alert
    }

    if ($config.Channels.Email) {
        Send-EmailAlert -Alert $Alert -EmailConfig $config.Email
    }

    if ($config.Channels.Slack) {
        Send-SlackAlert -Alert $Alert -SlackConfig $config.Slack
    }

    # 清理旧告警
    $cutoff = (Get-Date).AddDays(-7)
    Get-ChildItem $AlertDir -Filter "alert-*.json" |
        Where-Object { $_.LastWriteTime -lt $cutoff } |
        Remove-Item -Force
}

# 主执行
$alert = New-Alert -Type $Type -Severity $Severity -Message $Message -Details $Details
Send-Alert -Alert $alert
