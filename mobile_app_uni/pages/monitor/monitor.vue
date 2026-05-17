<template>
	<view class="page">
		<AppHeader title="智能物联监控" subtitle="OneNET 物模型实时数据">
			<template #right>
				<view class="device-state" :class="{ online: deviceOnline, offline: !deviceOnline && !loadingStatus }">
					<text class="state-dot"></text>
					<text>{{ deviceStateText }}</text>
				</view>
			</template>
		</AppHeader>

		<view class="summary-panel">
			<view class="summary-main">
				<text class="summary-label">设备</text>
				<text class="summary-title">{{ deviceName }}</text>
				<text class="summary-subtitle">Product ID：{{ settings.productId || '--' }}</text>
			</view>
			<button class="refresh-btn" :disabled="loading" @tap="loadDashboard(true)">
				<text>{{ loading ? '同步中' : '刷新' }}</text>
			</button>
		</view>

		<view class="metric-grid">
			<view
				v-for="sensor in sensors"
				:key="sensor.key"
				class="metric-card"
				:class="sensor.tone"
				@tap="openHistory(sensor)"
			>
				<view class="metric-head">
					<view class="metric-icon">
						<image v-if="sensor.iconPath" class="metric-icon-img" :src="sensor.iconPath" mode="aspectFit" />
					</view>
					<view class="metric-meta">
						<text class="metric-name">{{ sensor.label }}</text>
						<text class="metric-identifier">{{ getIdentifier(sensor.key) }}</text>
					</view>
				</view>
				<view class="metric-value-row">
					<text class="metric-value">{{ sensor.value }}</text>
					<text class="metric-unit">{{ sensor.unit }}</text>
				</view>
				<view class="metric-foot">
					<text>{{ sensor.status }}</text>
					<text>查看历史</text>
				</view>
			</view>
		</view>

		<view class="control-panel">
			<view class="control-head">
				<view>
					<text class="control-title">LED 远程控制</text>
					<text class="control-subtitle">点击后立即下发 OneNET 指令</text>
				</view>
				<view class="led-pill" :class="{ active: ledOn }">
					<text>{{ ledOn ? '已打开' : '已关闭' }}</text>
				</view>
			</view>
			<view class="control-actions">
				<button class="control-btn open" @tap="controlLed(true)">打开</button>
				<button class="control-btn close" @tap="controlLed(false)">关闭</button>
			</view>
			<text v-if="controlTip" class="control-tip">{{ controlTip }}</text>
		</view>

		<view v-if="errorMessage" class="error-card">
			<text class="error-title">请求提示</text>
			<text class="error-text">{{ errorMessage }}</text>
		</view>

		<CameraCard :enabled="settings.cameraEnabled" :camera-ip="settings.cameraIp" />

		<view v-if="historyVisible" class="history-mask" @tap="closeHistory">
			<view class="history-modal" @tap.stop>
				<view class="history-head">
					<view>
						<text class="history-title">{{ activeSensor.label }}历史数据</text>
						<text class="history-subtitle">最近 24 小时，最多 100 条</text>
					</view>
					<button class="history-close" @tap="closeHistory">×</button>
				</view>

				<view class="history-body">
					<view v-if="historyLoading" class="history-empty">
						<text>正在读取 OneNET 历史数据...</text>
					</view>
					<view v-else-if="historyError" class="history-empty warning">
						<text>{{ historyError }}</text>
					</view>
					<view v-else-if="historyRows.length" class="history-content">
						<view class="mini-chart">
							<view v-for="row in chartRows" :key="row.time + row.value" class="chart-item">
								<view class="chart-track">
									<view class="chart-fill" :style="{ height: `${row.level}%` }"></view>
								</view>
								<text class="chart-time">{{ row.time }}</text>
							</view>
						</view>
						<view class="history-table">
							<view class="history-row head">
								<text>时间</text>
								<text>数值</text>
								<text>状态</text>
							</view>
							<view v-for="row in historyRows" :key="row.time + row.value" class="history-row">
								<text>{{ row.time }}</text>
								<text>{{ row.value }}</text>
								<text class="ok-text">{{ row.status }}</text>
							</view>
						</view>
					</view>
					<view v-else class="history-empty">
						<text>暂无历史数据</text>
					</view>
				</view>
			</view>
		</view>

		<AppTabBar active="monitor" />
	</view>
</template>

<script>
import AppHeader from '../../components/AppHeader.vue'
import AppTabBar from '../../components/AppTabBar.vue'
import CameraCard from '../../components/CameraCard.vue'
import { loadSettings } from '../../utils/storage.js'
import {
	buildHistoryRows,
	buildMockMonitorData,
	buildOnenetMonitorData,
	getDeviceName,
	getPropertyIdentifier,
	queryDeviceProperties,
	queryDeviceStatus,
	queryPropertyHistory,
	setDeviceProperty
} from '../../utils/onenet.js'
import { appendHistorySnapshot } from '../../utils/history.js'

export default {
	components: {
		AppHeader,
		AppTabBar,
		CameraCard
	},
	data() {
		const mockData = buildMockMonitorData()
		return {
			settings: loadSettings(),
			sensors: mockData.sensors,
			ledOn: mockData.ledOn,
			deviceOnline: false,
			loading: false,
			loadingStatus: false,
			errorMessage: '',
			controlTip: '',
			controlTipTimer: null,
			pollingTimer: null,
			historyVisible: false,
			historyLoading: false,
			historyError: '',
			historyRows: [],
			activeSensor: {}
		}
	},
	computed: {
		deviceName() {
			return getDeviceName(this.settings) || '--'
		},
		deviceStateText() {
			if (this.settings.dataMode !== 'onenet') return 'Mock 模式'
			if (this.loadingStatus) return '检查中'
			return this.deviceOnline ? '在线' : '离线'
		},
		chartRows() {
			return this.historyRows.slice(-12)
		}
	},
	onShow() {
		this.settings = loadSettings()
		this.loadDashboard()
		this.startPolling()
	},
	onHide() {
		this.stopPolling()
	},
	onUnload() {
		this.stopPolling()
		if (this.controlTipTimer) clearTimeout(this.controlTipTimer)
	},
	onPullDownRefresh() {
		this.loadDashboard(true)
	},
	methods: {
		getIdentifier(key) {
			return getPropertyIdentifier(key)
		},
		startPolling() {
			this.stopPolling()
			this.pollingTimer = setInterval(() => {
				this.loadDashboard()
			}, 10000)
		},
		stopPolling() {
			if (this.pollingTimer) {
				clearInterval(this.pollingTimer)
				this.pollingTimer = null
			}
		},
		loadDashboard(isManual = false) {
			if (this.loading) {
				if (isManual) uni.stopPullDownRefresh()
				return
			}

			if (this.settings.dataMode !== 'onenet') {
				this.applyMonitorData(buildMockMonitorData())
				this.deviceOnline = true
				if (isManual) uni.stopPullDownRefresh()
				return
			}

			this.loading = true
			this.loadingStatus = true
			this.errorMessage = ''

			Promise.all([
				queryDeviceStatus(this.settings).catch(() => ({
					online: false
				})),
				queryDeviceProperties(this.settings)
			])
				.then(([status, items]) => {
					this.deviceOnline = status.online
					this.applyMonitorData(buildOnenetMonitorData(items))
				})
				.catch((error) => {
					this.deviceOnline = false
					this.errorMessage = error && error.message ? error.message : '从 OneNET 获取设备数据失败'
				})
				.then(() => {
					this.loading = false
					this.loadingStatus = false
					if (isManual) uni.stopPullDownRefresh()
				})
		},
		applyMonitorData(data) {
			this.sensors = data.sensors
			this.ledOn = data.ledOn
			this.errorMessage = data.errorMessage || ''
			if (this.settings.dataMode === 'onenet' && !this.errorMessage) {
				appendHistorySnapshot({
					sensors: this.sensors,
					ledOn: this.ledOn
				})
			}
		},
		controlLed(value) {
			this.ledOn = value
			this.controlTip = '已执行'
			if (this.controlTipTimer) clearTimeout(this.controlTipTimer)
			this.controlTipTimer = setTimeout(() => {
				this.controlTip = ''
			}, 1800)

			if (this.settings.dataMode !== 'onenet') return

			setDeviceProperty(this.settings, {
				led: value
			})
				.then(() => {
					this.loadDashboard()
				})
				.catch((error) => {
					console.warn('LED 控制指令发送失败', error)
				})
		},
		openHistory(sensor) {
			this.activeSensor = sensor
			this.historyVisible = true
			this.historyRows = []
			this.historyError = ''
			this.historyLoading = true

			if (this.settings.dataMode !== 'onenet') {
				this.historyLoading = false
				this.historyRows = []
				return
			}

			const endTime = Date.now()
			queryPropertyHistory(this.settings, getPropertyIdentifier(sensor.key), {
				startTime: endTime - 24 * 60 * 60 * 1000,
				endTime,
				limit: 100
			})
				.then((rows) => {
					this.historyRows = buildHistoryRows(sensor.key, rows)
					this.historyLoading = false
				})
				.catch((error) => {
					this.historyError = error && error.message ? error.message : '历史数据读取失败'
					this.historyLoading = false
				})
		},
		closeHistory() {
			this.historyVisible = false
		}
	}
}
</script>

<style>
.page {
	min-height: 100vh;
	padding: 32rpx 32rpx 168rpx;
	background: #F6F9FC;
	box-sizing: border-box;
}

.device-state {
	padding: 12rpx 18rpx;
	border-radius: 999rpx;
	background: #EEF2F7;
	color: #64748B;
	display: flex;
	align-items: center;
	gap: 8rpx;
	font-size: 24rpx;
	font-weight: 850;
	white-space: nowrap;
	border: 1rpx solid #E2E8F0;
}

.device-state.online {
	background: rgba(34, 197, 94, 0.14);
	color: #16A34A;
	border-color: rgba(34, 197, 94, 0.34);
}

.device-state.offline {
	background: rgba(239, 68, 68, 0.1);
	color: #EF4444;
	border-color: rgba(239, 68, 68, 0.3);
}

.state-dot {
	width: 12rpx;
	height: 12rpx;
	border-radius: 50%;
	background: currentColor;
}

.summary-panel,
.control-panel,
.error-card {
	border-radius: 28rpx;
	background: #FFFFFF;
	box-shadow: 0 16rpx 38rpx rgba(15, 23, 42, 0.065);
	border: 1rpx solid rgba(226, 232, 240, 0.72);
}

.summary-panel {
	margin-bottom: 22rpx;
	padding: 26rpx;
	display: flex;
	align-items: center;
	justify-content: space-between;
	gap: 20rpx;
}

.summary-main {
	min-width: 0;
}

.summary-label,
.summary-title,
.summary-subtitle {
	display: block;
}

.summary-label {
	font-size: 22rpx;
	color: #64748B;
}

.summary-title {
	margin-top: 6rpx;
	font-size: 34rpx;
	line-height: 1.2;
	font-weight: 900;
	color: #0F172A;
}

.summary-subtitle {
	margin-top: 6rpx;
	font-size: 22rpx;
	color: #94A3B8;
}

.refresh-btn {
	margin: 0;
	width: 132rpx;
	height: 68rpx;
	line-height: 68rpx;
	border-radius: 999rpx;
	background: #0F172A;
	color: #FFFFFF;
	font-size: 24rpx;
	font-weight: 850;
}

.refresh-btn::after,
.control-btn::after,
.history-close::after {
	border: 0;
}

.metric-grid {
	display: flex;
	flex-wrap: wrap;
	gap: 18rpx;
}

.metric-card {
	width: calc(50% - 9rpx);
	min-height: 226rpx;
	padding: 24rpx;
	border-radius: 28rpx;
	background: #FFFFFF;
	border: 1rpx solid rgba(226, 232, 240, 0.82);
	box-shadow: 0 14rpx 32rpx rgba(15, 23, 42, 0.055);
	box-sizing: border-box;
}

.metric-head {
	display: flex;
	align-items: center;
	gap: 16rpx;
}

.metric-icon {
	width: 64rpx;
	height: 64rpx;
	border-radius: 20rpx;
	background: #EFF6FF;
	display: flex;
	align-items: center;
	justify-content: center;
}

.metric-icon-img {
	width: 48rpx;
	height: 48rpx;
}

.metric-meta {
	flex: 1;
	min-width: 0;
}

.metric-name,
.metric-identifier {
	display: block;
}

.metric-name {
	font-size: 28rpx;
	font-weight: 900;
	color: #0F172A;
}

.metric-identifier {
	margin-top: 4rpx;
	font-size: 20rpx;
	color: #94A3B8;
}

.metric-value-row {
	margin-top: 26rpx;
	display: flex;
	align-items: baseline;
	gap: 8rpx;
}

.metric-value {
	font-size: 46rpx;
	line-height: 1;
	font-weight: 900;
	color: #0F172A;
}

.metric-unit {
	font-size: 22rpx;
	font-weight: 750;
	color: #64748B;
}

.metric-foot {
	margin-top: 22rpx;
	display: flex;
	align-items: center;
	justify-content: space-between;
	font-size: 22rpx;
	color: #64748B;
}

.metric-card.teal .metric-icon {
	background: #ECFDF5;
}

.metric-card.indigo .metric-icon {
	background: #EEF2FF;
}

.metric-card.amber .metric-icon {
	background: #FFF7ED;
}

.control-panel {
	margin-top: 24rpx;
	padding: 28rpx;
}

.control-head {
	display: flex;
	align-items: flex-start;
	justify-content: space-between;
	gap: 20rpx;
}

.control-title,
.control-subtitle {
	display: block;
}

.control-title {
	font-size: 30rpx;
	font-weight: 900;
	color: #0F172A;
}

.control-subtitle {
	margin-top: 6rpx;
	font-size: 22rpx;
	color: #94A3B8;
}

.led-pill {
	padding: 10rpx 18rpx;
	border-radius: 999rpx;
	background: #F1F5F9;
	color: #64748B;
	font-size: 22rpx;
	font-weight: 850;
	white-space: nowrap;
}

.led-pill.active {
	background: rgba(34, 197, 94, 0.14);
	color: #16A34A;
}

.control-actions {
	margin-top: 24rpx;
	display: flex;
	gap: 18rpx;
}

.control-btn {
	flex: 1;
	margin: 0;
	height: 82rpx;
	line-height: 82rpx;
	border-radius: 22rpx;
	color: #FFFFFF;
	font-size: 28rpx;
	font-weight: 900;
}

.control-btn.open {
	background: #16A34A;
}

.control-btn.close {
	background: #EF4444;
}

.control-tip {
	display: block;
	margin-top: 18rpx;
	text-align: center;
	font-size: 24rpx;
	font-weight: 850;
	color: #2563EB;
}

.error-card {
	margin-top: 24rpx;
	padding: 22rpx 26rpx;
	display: flex;
	flex-direction: column;
	gap: 8rpx;
	background: rgba(239, 68, 68, 0.08);
	border-color: rgba(239, 68, 68, 0.16);
}

.error-title {
	font-size: 24rpx;
	font-weight: 900;
	color: #EF4444;
}

.error-text {
	font-size: 22rpx;
	line-height: 1.5;
	color: #B91C1C;
}

.history-mask {
	position: fixed;
	left: 0;
	right: 0;
	top: 0;
	bottom: 0;
	z-index: 50;
	padding: 48rpx 28rpx;
	background: rgba(15, 23, 42, 0.46);
	display: flex;
	align-items: center;
	justify-content: center;
	box-sizing: border-box;
}

.history-modal {
	width: 100%;
	max-width: 900rpx;
	max-height: 86vh;
	border-radius: 30rpx;
	background: #FFFFFF;
	overflow: hidden;
	display: flex;
	flex-direction: column;
}

.history-head {
	padding: 28rpx;
	border-bottom: 1rpx solid #EEF2F7;
	display: flex;
	align-items: flex-start;
	justify-content: space-between;
	gap: 20rpx;
}

.history-title,
.history-subtitle {
	display: block;
}

.history-title {
	font-size: 32rpx;
	font-weight: 900;
	color: #0F172A;
}

.history-subtitle {
	margin-top: 6rpx;
	font-size: 22rpx;
	color: #94A3B8;
}

.history-close {
	margin: 0;
	padding: 0;
	width: 58rpx;
	height: 58rpx;
	line-height: 54rpx;
	border-radius: 50%;
	background: #F1F5F9;
	color: #334155;
	font-size: 40rpx;
}

.history-body {
	padding: 28rpx;
	overflow: auto;
}

.mini-chart {
	height: 300rpx;
	padding: 28rpx 18rpx 18rpx;
	border-radius: 24rpx;
	background: #F8FAFC;
	border: 1rpx solid #E2E8F0;
	display: flex;
	align-items: flex-end;
	gap: 14rpx;
	box-sizing: border-box;
}

.chart-item {
	flex: 1;
	height: 100%;
	display: flex;
	flex-direction: column;
	align-items: center;
	justify-content: flex-end;
	gap: 10rpx;
	min-width: 0;
}

.chart-track {
	position: relative;
	width: 100%;
	max-width: 30rpx;
	flex: 1;
	border-radius: 999rpx;
	background: #E2E8F0;
	overflow: hidden;
}

.chart-fill {
	position: absolute;
	left: 0;
	right: 0;
	bottom: 0;
	border-radius: 999rpx;
	background: #2563EB;
}

.chart-time {
	font-size: 18rpx;
	color: #94A3B8;
}

.history-table {
	margin-top: 22rpx;
}

.history-row {
	min-height: 72rpx;
	display: flex;
	align-items: center;
	border-bottom: 1rpx solid #EEF2F7;
	font-size: 24rpx;
	color: #334155;
}

.history-row text {
	flex: 1;
}

.history-row.head {
	font-weight: 900;
	color: #64748B;
}

.ok-text {
	color: #16A34A;
	font-weight: 850;
}

.history-empty {
	min-height: 280rpx;
	display: flex;
	align-items: center;
	justify-content: center;
	text-align: center;
	color: #64748B;
	font-size: 26rpx;
}

.history-empty.warning {
	color: #B45309;
}

@media screen and (min-width: 900px) {
	.page {
		max-width: 1180px;
		margin: 0 auto;
	}

	.metric-card {
		width: calc(25% - 14rpx);
	}
}
</style>
