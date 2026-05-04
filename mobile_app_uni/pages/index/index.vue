<template>
	<view class="page">
		<view class="hero">
			<view class="hero-topline">OneNet 设备看板</view>
			<view class="hero-title">环境监测终端</view>
			<view class="hero-subtitle">实时查看温度、湿度、气压和光照强度，并远程控制板载 LED。</view>
		</view>

		<view class="overview-panel">
			<view class="overview-item">
				<text class="overview-label">连接状态</text>
				<text class="status-value" :class="{ online: !loading && !errorMessage, error: !!errorMessage }">
					{{ statusText }}
				</text>
			</view>
			<view class="overview-divider"></view>
			<view class="overview-item align-right">
				<text class="overview-label">最近同步</text>
				<text class="overview-value">{{ lastSyncText }}</text>
			</view>
		</view>

		<view class="section-head">
			<view class="section-title">环境数据</view>
			<view class="section-note">单位按当前物模型显示</view>
		</view>

		<view class="card-grid">
			<view
				v-for="metric in metrics"
				:key="metric.key"
				class="metric-card"
				:class="metric.tone"
			>
				<view class="metric-head">
					<view>
						<view class="metric-name">{{ metric.label }}</view>
						<view class="metric-desc">{{ metric.desc }}</view>
					</view>
					<view class="metric-icon-badge" :class="metric.tone">
						<text class="metric-icon-symbol">{{ metric.symbol }}</text>
					</view>
				</view>
				<view class="metric-value-row">
					<text class="metric-value">{{ metric.value }}</text>
					<text class="metric-unit">{{ metric.unit }}</text>
				</view>
			</view>
		</view>

		<view class="section-head control-head">
			<view class="section-title">设备控制</view>
			<view class="section-note">写入 OneNet 属性</view>
		</view>

		<view class="control-panel">
			<view class="control-main">
				<view class="control-icon-badge">
					<text class="metric-icon-symbol">灯</text>
				</view>
				<view class="control-copy">
					<view class="control-title">板载 LED</view>
					<view class="control-subtitle">
						当前状态：
						<text class="control-state" :class="{ active: LED }">{{ LED ? '开启' : '关闭' }}</text>
					</view>
				</view>
			</view>
			<switch :checked="LED" :disabled="ledSubmitting" @change="onLEDSwitch" color="#0f9d58" />
		</view>

		<view v-if="errorMessage" class="error-box">
			<text class="error-title">请求失败</text>
			<text class="error-text">{{ errorMessage }}</text>
		</view>
	</view>
</template>

<script>
export default {
	data() {
		return {
			humidity: null,
			temperature: null,
			light: null,
			pressure: null,
			LED: false,
			loading: false,
			ledSubmitting: false,
			errorMessage: '',
			lastSyncText: '--',
			pollingTimer: null,
			pollingIntervalMs: 2000,
			apiConfig: {
				productId: '45Z51DlJn3',
				deviceName: 'stm32-esp32c3',
				token: 'version=2018-10-31&res=products%2F45Z51DlJn3%2Fdevices%2Fstm32-esp32c3&et=1809399480&method=md5&sign=QAfZy6%2F2oq090K%2B043BVrA%3D%3D'
			}
		}
	},
	computed: {
		humidityDisplay() {
			return this.formatMetric(this.humidity, 1)
		},
		temperatureDisplay() {
			return this.formatMetric(this.temperature, 1)
		},
		lightDisplay() {
			return this.formatMetric(this.light, 1)
		},
		pressureDisplay() {
			return this.formatMetric(this.pressure, 2)
		},
		metrics() {
			return [
				{ key: 'humi', label: '湿度', desc: '空气含湿量', value: this.humidityDisplay, unit: '%RH', symbol: '湿', tone: 'tone-blue' },
				{ key: 'temp', label: '温度', desc: '环境温度', value: this.temperatureDisplay, unit: '°C', symbol: '温', tone: 'tone-orange' },
				{ key: 'press', label: '气压', desc: '大气压强', value: this.pressureDisplay, unit: 'hPa', symbol: '压', tone: 'tone-green' },
				{ key: 'light', label: '光照', desc: '光照强度', value: this.lightDisplay, unit: '%', symbol: '光', tone: 'tone-gold' }
			]
		},
		statusText() {
			if (this.errorMessage) {
				return '异常'
			}
			if (this.loading) {
				return '同步中'
			}
			return '已连接'
		}
	},
	onShow() {
		this.fetchDeviceData()
		this.startPolling()
	},
	onHide() {
		this.stopPolling()
	},
	onUnload() {
		this.stopPolling()
	},
	methods: {
		formatMetric(value, digits) {
			if (value === null || value === undefined || value === '--' || value === '') {
				return '--'
			}

			const numericValue = Number(value)
			return Number.isFinite(numericValue) ? numericValue.toFixed(digits) : String(value)
		},
		formatTime(timestamp) {
			if (!timestamp || !Number.isFinite(Number(timestamp))) {
				return '--'
			}

			const date = new Date(Number(timestamp))
			const pad = (value) => String(value).padStart(2, '0')
			return `${pad(date.getHours())}:${pad(date.getMinutes())}:${pad(date.getSeconds())}`
		},
		startPolling() {
			this.stopPolling()
			this.pollingTimer = setInterval(() => {
				this.fetchDeviceData()
			}, this.pollingIntervalMs)
		},
		stopPolling() {
			if (this.pollingTimer) {
				clearInterval(this.pollingTimer)
				this.pollingTimer = null
			}
		},
		getPropertyItem(items, identifier) {
			return items.find((entry) => entry.identifier === identifier) || null
		},
		normalizeValue(value) {
			if (value && typeof value === 'object' && 'value' in value) {
				return value.value
			}
			return value
		},
		parseBoolean(value) {
			const normalizedValue = this.normalizeValue(value)
			if (typeof normalizedValue === 'boolean') {
				return normalizedValue
			}
			if (typeof normalizedValue === 'string') {
				return normalizedValue.toLowerCase() === 'true'
			}
			if (typeof normalizedValue === 'number') {
				return normalizedValue !== 0
			}
			return Boolean(normalizedValue)
		},
		getApiMessage(response, fallbackMessage) {
			const message = response && response.data && response.data.msg
			return message || fallbackMessage
		},
		updateDeviceState(items) {
			const humidityItem = this.getPropertyItem(items, 'Humi')
			const temperatureItem = this.getPropertyItem(items, 'Temp')
			const lightItem = this.getPropertyItem(items, 'Light')
			const pressureItem = this.getPropertyItem(items, 'Press')
			const ledItem = this.getPropertyItem(items, 'led')

			this.humidity = humidityItem ? this.normalizeValue(humidityItem.value) : null
			this.temperature = temperatureItem ? this.normalizeValue(temperatureItem.value) : null
			this.light = lightItem ? this.normalizeValue(lightItem.value) : null
			this.pressure = pressureItem ? this.normalizeValue(pressureItem.value) : null
			this.LED = this.parseBoolean(ledItem ? ledItem.value : false)

			const latestTimestamp = [humidityItem, temperatureItem, lightItem, pressureItem, ledItem]
				.filter(Boolean)
				.map((item) => Number(item.time) || 0)
				.reduce((max, current) => Math.max(max, current), 0)

			this.lastSyncText = this.formatTime(latestTimestamp)
		},
		fetchDeviceData() {
			this.loading = true
			this.errorMessage = ''

			uni.request({
				url: 'https://iot-api.heclouds.com/thingmodel/query-device-property',
				method: 'GET',
				data: {
					product_id: this.apiConfig.productId,
					device_name: this.apiConfig.deviceName
				},
				header: {
					authorization: this.apiConfig.token
				},
				success: (res) => {
					if (!res || !res.data || res.data.code !== 0) {
						this.errorMessage = this.getApiMessage(res, '从 OneNet 获取设备数据失败。')
						return
					}

					const items = res.data.data
					if (!Array.isArray(items)) {
						this.errorMessage = '设备返回格式不符合预期。'
						return
					}

					this.updateDeviceState(items)
				},
				fail: (err) => {
					console.error('Failed to fetch device data', err)
					this.errorMessage = '从 OneNet 获取设备数据失败。'
				},
				complete: () => {
					this.loading = false
				}
			})
		},
		onLEDSwitch(event) {
			const ledValue = event.detail.value
			const previousValue = this.LED
			this.LED = ledValue
			this.ledSubmitting = true
			this.errorMessage = ''

			uni.request({
				url: 'https://iot-api.heclouds.com/thingmodel/set-device-property',
				method: 'POST',
				data: {
					product_id: this.apiConfig.productId,
					device_name: this.apiConfig.deviceName,
					params: {
						led: ledValue
					}
				},
				header: {
					authorization: this.apiConfig.token
				},
				success: (res) => {
					if (!res || !res.data || res.data.code !== 0) {
						this.LED = previousValue
						this.errorMessage = this.getApiMessage(res, '写入 LED 状态失败。')
						uni.showToast({
							title: this.errorMessage,
							icon: 'none'
						})
						return
					}

					uni.showToast({
						title: `LED 已${ledValue ? '开启' : '关闭'}`,
						icon: 'none'
					})
				},
				fail: (err) => {
					console.error('Failed to control LED', err)
					this.LED = previousValue
					this.errorMessage = '写入 LED 状态失败。'
					uni.showToast({
						title: 'LED 控制失败',
						icon: 'none'
					})
				},
				complete: () => {
					this.ledSubmitting = false
				}
			})
		}
	}
}
</script>

<style>
.page {
	min-height: 100vh;
	padding: 20rpx;
	background:
		radial-gradient(circle at top right, rgba(15, 157, 88, 0.12), transparent 28%),
		radial-gradient(circle at top left, rgba(29, 125, 250, 0.14), transparent 30%),
		linear-gradient(180deg, #eff6f2 0%, #f7fbf8 32%, #ffffff 100%);
}

.hero {
	padding: 24rpx;
	border-radius: 28rpx;
	background: linear-gradient(140deg, #10344b 0%, #18607c 48%, #0f9d58 100%);
	box-shadow: 0 18rpx 40rpx rgba(14, 56, 73, 0.18);
}

.hero-topline {
	color: rgba(255, 255, 255, 0.74);
	font-size: 20rpx;
	letter-spacing: 2rpx;
}

.hero-title {
	margin-top: 10rpx;
	color: #ffffff;
	font-size: 42rpx;
	font-weight: 700;
}

.hero-subtitle {
	margin-top: 12rpx;
	color: rgba(255, 255, 255, 0.82);
	font-size: 22rpx;
	line-height: 1.7;
}

.overview-panel {
	margin-top: 18rpx;
	padding: 22rpx 24rpx;
	border-radius: 22rpx;
	background: rgba(255, 255, 255, 0.92);
	display: flex;
	justify-content: space-between;
	align-items: center;
	box-shadow: 0 12rpx 26rpx rgba(16, 52, 75, 0.07);
}

.overview-item {
	flex: 1;
	display: flex;
	flex-direction: column;
	gap: 8rpx;
}

.align-right {
	align-items: flex-end;
}

.overview-label {
	font-size: 20rpx;
	color: #6b7f74;
}

.overview-value,
.status-value {
	font-size: 22rpx;
	font-weight: 600;
	color: #163347;
}

.overview-divider {
	width: 2rpx;
	height: 54rpx;
	background: rgba(16, 52, 75, 0.08);
	margin: 0 20rpx;
}

.status-value.error {
	color: #d13f3f;
}

.status-value.online {
	color: #0f9d58;
}

.section-head {
	margin-top: 14rpx;
	padding: 6rpx 6rpx 0;
	display: flex;
	align-items: center;
	justify-content: space-between;
}

.section-title {
	font-size: 28rpx;
	font-weight: 700;
	color: #18384d;
}

.section-note {
	font-size: 18rpx;
	color: #7a8f84;
}

.card-grid {
	margin-top: 12rpx;
	display: grid;
	grid-template-columns: repeat(2, minmax(0, 1fr));
	gap: 12rpx;
}

.metric-card {
	min-height: 188rpx;
	padding: 20rpx;
	border-radius: 24rpx;
	background: rgba(255, 255, 255, 0.98);
	box-shadow: 0 12rpx 30rpx rgba(17, 44, 61, 0.08);
	display: flex;
	flex-direction: column;
	justify-content: space-between;
	position: relative;
	overflow: hidden;
}

.metric-card::after {
	content: '';
	position: absolute;
	right: -30rpx;
	bottom: -34rpx;
	width: 120rpx;
	height: 120rpx;
	border-radius: 50%;
	opacity: 0.12;
}

.tone-blue::after {
	background: #2d7ff9;
}

.tone-orange::after {
	background: #ff8a3d;
}

.tone-green::after {
	background: #10a86b;
}

.tone-gold::after {
	background: #d9a21b;
}

.metric-head {
	display: flex;
	align-items: center;
	justify-content: space-between;
	gap: 12rpx;
}

.metric-name {
	font-size: 26rpx;
	font-weight: 600;
	color: #1e3a4d;
}

.metric-desc {
	margin-top: 6rpx;
	font-size: 18rpx;
	color: #7b8b96;
}

.metric-icon-badge,
.control-icon-badge {
	width: 64rpx;
	height: 64rpx;
	border-radius: 20rpx;
	display: flex;
	align-items: center;
	justify-content: center;
	position: relative;
	z-index: 1;
}

.metric-icon-badge.tone-blue {
	background: linear-gradient(135deg, rgba(45, 127, 249, 0.16), rgba(45, 127, 249, 0.26));
}

.metric-icon-badge.tone-orange {
	background: linear-gradient(135deg, rgba(255, 138, 61, 0.16), rgba(255, 138, 61, 0.26));
}

.metric-icon-badge.tone-green {
	background: linear-gradient(135deg, rgba(16, 168, 107, 0.16), rgba(16, 168, 107, 0.26));
}

.metric-icon-badge.tone-gold {
	background: linear-gradient(135deg, rgba(217, 162, 27, 0.16), rgba(217, 162, 27, 0.26));
}

.control-icon-badge {
	background: linear-gradient(135deg, rgba(15, 157, 88, 0.16), rgba(15, 157, 88, 0.28));
}

.metric-icon-symbol {
	font-size: 28rpx;
	font-weight: 700;
	color: #163347;
}

.metric-value-row {
	margin-top: 22rpx;
	display: flex;
	align-items: flex-end;
	gap: 10rpx;
	position: relative;
	z-index: 1;
}

.metric-value {
	font-size: 44rpx;
	line-height: 1;
	font-weight: 700;
	color: #132534;
}

.metric-unit {
	font-size: 20rpx;
	color: #63788c;
	padding-bottom: 6rpx;
}

.control-head {
	margin-top: 16rpx;
}

.control-panel {
	margin-top: 12rpx;
	padding: 22rpx 24rpx;
	border-radius: 24rpx;
	background: linear-gradient(135deg, rgba(255, 255, 255, 0.98), rgba(241, 250, 245, 0.96));
	box-shadow: 0 14rpx 28rpx rgba(17, 44, 61, 0.08);
	display: flex;
	align-items: center;
	justify-content: space-between;
	gap: 16rpx;
}

.control-main {
	display: flex;
	align-items: center;
	gap: 16rpx;
}

.control-copy {
	display: flex;
	flex-direction: column;
	gap: 8rpx;
}

.control-title {
	font-size: 28rpx;
	font-weight: 700;
	color: #163347;
}

.control-subtitle {
	font-size: 20rpx;
	color: #6e8174;
}

.control-state {
	color: #7a8894;
	font-weight: 600;
}

.control-state.active {
	color: #0f9d58;
}

.error-box {
	margin-top: 16rpx;
	padding: 18rpx 20rpx;
	border-radius: 20rpx;
	background: rgba(209, 63, 63, 0.08);
	display: flex;
	flex-direction: column;
	gap: 6rpx;
}

.error-title {
	font-size: 22rpx;
	font-weight: 700;
	color: #bf2f2f;
}

.error-text {
	font-size: 20rpx;
	line-height: 1.6;
	color: #c64a4a;
}

@media (max-width: 560px) {
	.page {
		padding: 16rpx;
	}

	.hero-title {
		font-size: 38rpx;
	}

	.metric-card {
		min-height: 174rpx;
		padding: 18rpx;
	}

	.metric-value {
		font-size: 38rpx;
	}

	.control-panel {
		padding: 20rpx;
	}
}
</style>
