# DashboardServer.ps1 - 仪表板Web服务器
param(
    [int]$Port = 8080,
    [string]$DashboardPath = "dashboard.html"
)

$ErrorActionPreference = "Stop"

Write-Host "Starting Dashboard Web Server..." -ForegroundColor Green
Write-Host "Port: $Port"
Write-Host "Dashboard: $DashboardPath"
Write-Host "URL: http://localhost:$Port/"
Write-Host "Press Ctrl+C to stop"
Write-Host ""

# 启动自动更新
$updateScript = Join-Path $PSScriptRoot "UpdateDashboard.ps1"
$updateJob = Start-Job -ScriptBlock {
    param($Script, $Path)
    & $Script -DashboardPath $Path
} -ArgumentList $updateScript, $DashboardPath

Write-Host "Dashboard auto-update started (Job ID: $($updateJob.Id))" -ForegroundColor Cyan

try {
    # 创建HTTP监听器
    $listener = New-Object System.Net.HttpListener
    $listener.Prefixes.Add("http://localhost:$Port/")
    $listener.Start()

    Write-Host "Server is running..." -ForegroundColor Green
    Write-Host ""

    while ($listener.IsListening) {
        $context = $listener.GetContext()
        $request = $context.Request
        $response = $context.Response

        Write-Host "[$(Get-Date -Format 'HH:mm:ss')] $($request.HttpMethod) $($request.Url.LocalPath)" -ForegroundColor Gray

        try {
            $dashboardFile = Join-Path $PSScriptRoot $DashboardPath

            if (Test-Path $dashboardFile) {
                $content = Get-Content $dashboardFile -Raw -Encoding UTF8
                $buffer = [System.Text.Encoding]::UTF8.GetBytes($content)

                $response.ContentType = "text/html; charset=utf-8"
                $response.ContentLength64 = $buffer.Length
                $response.OutputStream.Write($buffer, 0, $buffer.Length)
            } else {
                $errorMsg = "Dashboard not found. Please run GenerateDashboard.ps1 first."
                $buffer = [System.Text.Encoding]::UTF8.GetBytes($errorMsg)

                $response.StatusCode = 404
                $response.ContentType = "text/plain; charset=utf-8"
                $response.ContentLength64 = $buffer.Length
                $response.OutputStream.Write($buffer, 0, $buffer.Length)
            }

        } catch {
            Write-Host "Error serving request: $_" -ForegroundColor Red

            $errorMsg = "Internal Server Error: $_"
            $buffer = [System.Text.Encoding]::UTF8.GetBytes($errorMsg)

            $response.StatusCode = 500
            $response.ContentType = "text/plain; charset=utf-8"
            $response.ContentLength64 = $buffer.Length
            $response.OutputStream.Write($buffer, 0, $buffer.Length)
        } finally {
            $response.Close()
        }
    }

} finally {
    Write-Host "`nShutting down server..." -ForegroundColor Yellow

    if ($listener) {
        $listener.Stop()
        $listener.Close()
    }

    if ($updateJob) {
        Stop-Job -Job $updateJob
        Remove-Job -Job $updateJob
        Write-Host "Dashboard auto-update stopped" -ForegroundColor Yellow
    }

    Write-Host "Server stopped" -ForegroundColor Green
}
