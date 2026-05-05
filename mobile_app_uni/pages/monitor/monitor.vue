<template>
	<view class="page">
		<AppHeader title="传感器监测" subtitle="实时环境状态">
			<template #right>
				<view class="device-state" :class="{ offline: !!errorMessage }">
					<text class="state-dot"></text>
					<text>{{ deviceStateText }}</text>
				</view>
			</template>
		</AppHeader>

		<view class="sensor-list">
			<SensorCard
				v-for="sensor in sensors"
				:key="sensor.key"
				:name="sensor.label"
				:value="sensor.value"
				:unit="sensor.unit"
				:status="sensor.status"
				:icon="sensor.icon"
				:icon-path="sensor.iconPath"
				:tone="sensor.tone"
			/>
			<SensorCard
				name="LED 状态"
				:value="ledOn"
				:status="ledStatus"
				icon="💡"
				icon-path="/static/icons/led_status_control_c.png"
				tone="warning"
				switchable
				:checked="ledOn"
				@toggle="handleLedToggle"
			/>
		</view>

		<view v-if="errorMessage" class="error-card">
			<text class="error-title">请求失败</text>
			<text class="error-text">{{ errorMessage }}</text>
		</view>

		<AppTabBar active="monitor" />
	</view>
</template>

<script>
import AppHeader from '../../components/AppHeader.vue'
import AppTabBar from '../../components/AppTabBar.vue'
import SensorCard from '../../components/SensorCard.vue'
import { loadSettings } from '../../utils/storage.js'
import {
	buildMockMonitorData,
	buildOnenetMonitorData,
	queryDeviceProperties,
	setDeviceProperty
} from '../../utils/onenet.js'

export default {
	components: {
		AppHeader,
		AppTabBar,
		SensorCard
	},
	data() {
		const mockData = buildMockMonitorData()
		return {
			settings: loadSettings(),
			sensors: mockData.sensors,
			ledOn: mockData.ledOn,
			ledSubmitting: false,
			errorMessage: ''
		}
	},
	computed: {
		deviceStateText() {
			if (this.errorMessage) return '连接失败'
			return this.settings.dataMode === 'onenet' ? '设备在线' : 'Mock 模式'
		},
		ledStatus() {
			if (this.ledSubmitting) return '同步中'
			return this.errorMessage ? '异常' : '正常'
		}
	},
	onShow() {
		this.settings = loadSettings()
		this.loadMonitorData()
	},
	methods: {
		applyMonitorData(data) {
			this.sensors = data.sensors
			this.ledOn = data.ledOn
			this.errorMessage = data.errorMessage || ''
		},
		loadMonitorData() {
			if (this.settings.dataMode !== 'onenet') {
				this.applyMonitorData(buildMockMonitorData())
				return
			}

			this.errorMessage = ''
			queryDeviceProperties(this.settings)
				.then((items) => {
					this.applyMonitorData(buildOnenetMonitorData(items))
				})
				.catch((error) => {
					this.errorMessage = error && error.message ? error.message : '从 OneNet 获取设备数据失败。'
				})
		},
		handleLedToggle(value) {
			const previousValue = this.ledOn
			this.ledOn = value

			if (this.settings.dataMode !== 'onenet') {
				return
			}

			this.ledSubmitting = true
			this.errorMessage = ''
			setDeviceProperty(this.settings, {
				led: value
			})
				.then(() => {
					uni.showToast({
						title: `LED 已${value ? '开启' : '关闭'}`,
						icon: 'none'
					})
					this.ledSubmitting = false
				})
				.catch((error) => {
					this.ledOn = previousValue
					this.errorMessage = error && error.message ? error.message : '写入 LED 状态失败。'
					uni.showToast({
						title: 'LED 控制失败',
						icon: 'none'
					})
					this.ledSubmitting = false
				})
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
	background: rgba(34, 197, 94, 0.14);
	color: #16A34A;
	display: flex;
	align-items: center;
	gap: 8rpx;
	font-size: 24rpx;
	font-weight: 850;
	white-space: nowrap;
	border: 1rpx solid rgba(34, 197, 94, 0.34);
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
	box-shadow: 0 0 0 6rpx rgba(34, 197, 94, 0.14);
}

.sensor-list {
	display: flex;
	flex-wrap: wrap;
	gap: 18rpx;
}

.sensor-list .sensor-card {
	width: calc(50% - 9rpx);
}

.error-card {
	margin-top: 24rpx;
	padding: 22rpx 26rpx;
	border-radius: 28rpx;
	background: rgba(239, 68, 68, 0.08);
	border: 1rpx solid rgba(239, 68, 68, 0.16);
	display: flex;
	flex-direction: column;
	gap: 8rpx;
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
</style>
