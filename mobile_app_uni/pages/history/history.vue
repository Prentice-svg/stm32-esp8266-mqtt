<template>
	<view class="page">
		<AppHeader title="历史数据" subtitle="查看 OneNET 物模型属性趋势" />

		<view class="point-list">
			<view
				v-for="point in points"
				:key="point.key"
				class="point-row"
				:class="{ active: activeKey === point.key }"
				@tap="selectPoint(point.key)"
			>
				<image class="point-icon" :src="point.iconPath" mode="aspectFit" />
				<view class="point-main">
					<text class="point-name">{{ point.label }}</text>
					<text class="point-desc">{{ getIdentifier(point.key) }} · {{ point.unit }}</text>
				</view>
				<text class="point-arrow">›</text>
			</view>
		</view>

		<view class="history-card">
			<view class="card-head">
				<view>
					<text class="card-title">{{ activePoint.label }}趋势</text>
					<text class="card-subtitle">{{ historySubtitle }}</text>
				</view>
				<view class="period-tabs">
					<text
						v-for="item in periods"
						:key="item.key"
						class="period-tab"
						:class="{ active: period === item.key }"
						@tap="selectPeriod(item.key)"
					>
						{{ item.text }}
					</text>
				</view>
			</view>

			<view v-if="loadingHistory" class="empty-box">
				<text>正在读取 OneNET 历史数据...</text>
			</view>
			<view v-else-if="historyError" class="empty-box warning">
				<text>{{ historyError }}</text>
			</view>
			<view v-else-if="activeRows.length">
				<view class="chart-card">
					<view v-for="item in chartRows" :key="item.time + item.value" class="bar-item">
						<view class="bar-track">
							<view class="bar-fill" :style="{ height: `${item.level}%` }"></view>
						</view>
						<text class="bar-time">{{ item.time }}</text>
					</view>
				</view>

				<view class="record-table">
					<view class="record-row head">
						<text>时间</text>
						<text>数值</text>
						<text>状态</text>
					</view>
					<view v-for="row in activeRows" :key="row.time + row.value" class="record-row">
						<text>{{ row.time }}</text>
						<text>{{ row.value }}</text>
						<text class="success-text">{{ row.status }}</text>
					</view>
				</view>
			</view>
			<view v-else class="empty-box">
				<text>暂无历史数据</text>
			</view>
		</view>

		<AppTabBar active="history" />
	</view>
</template>

<script>
import AppHeader from '../../components/AppHeader.vue'
import AppTabBar from '../../components/AppTabBar.vue'
import { sensorList, historyMap } from '../../utils/mockData.js'
import { loadHistoryMap } from '../../utils/history.js'
import { loadSettings } from '../../utils/storage.js'
import { buildHistoryRows, getPropertyIdentifier, queryPropertyHistory } from '../../utils/onenet.js'

export default {
	components: {
		AppHeader,
		AppTabBar
	},
	data() {
		return {
			settings: loadSettings(),
			points: sensorList,
			localHistory: {},
			remoteRows: [],
			historyError: '',
			loadingHistory: false,
			activeKey: 'temperature',
			period: 'day',
			periods: [
				{ key: 'day', text: '日' },
				{ key: 'week', text: '周' },
				{ key: 'month', text: '月' }
			]
		}
	},
	onShow() {
		this.settings = loadSettings()
		this.localHistory = loadHistoryMap(false)
		this.loadRemoteHistory()
	},
	computed: {
		activePoint() {
			return this.points.find((item) => item.key === this.activeKey) || this.points[0]
		},
		activeRows() {
			if (this.remoteRows.length) return this.remoteRows
			const rows = this.localHistory[this.activeKey] || []
			return rows.length ? rows : (historyMap[this.activeKey] || [])
		},
		chartRows() {
			return this.activeRows.slice(-12)
		},
		historySubtitle() {
			if (this.loadingHistory) return '正在读取 OneNET'
			if (this.remoteRows.length) return 'OneNET 历史记录'
			return '本地记录或示例数据'
		}
	},
	methods: {
		getIdentifier(key) {
			return getPropertyIdentifier(key)
		},
		selectPoint(key) {
			if (this.activeKey === key) return
			this.activeKey = key
			this.loadRemoteHistory()
		},
		selectPeriod(period) {
			if (this.period === period) return
			this.period = period
			this.loadRemoteHistory()
		},
		getRange() {
			const endTime = Date.now()
			const durationMap = {
				day: 24 * 60 * 60 * 1000,
				week: 7 * 24 * 60 * 60 * 1000,
				month: 30 * 24 * 60 * 60 * 1000
			}
			return {
				startTime: endTime - (durationMap[this.period] || durationMap.day),
				endTime,
				limit: 100
			}
		},
		loadRemoteHistory() {
			this.remoteRows = []
			this.historyError = ''

			if (!this.settings || this.settings.dataMode !== 'onenet') {
				return
			}

			const identifier = getPropertyIdentifier(this.activeKey)
			if (!identifier) return

			this.loadingHistory = true
			queryPropertyHistory(this.settings, identifier, this.getRange())
				.then((rows) => {
					this.remoteRows = buildHistoryRows(this.activeKey, rows)
					this.loadingHistory = false
				})
				.catch((error) => {
					this.historyError = error && error.message ? error.message : 'OneNET 历史读取失败'
					this.loadingHistory = false
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
	display: flex;
	flex-direction: column;
	gap: 24rpx;
}

.point-list,
.history-card {
	border-radius: 28rpx;
	background: #FFFFFF;
	box-shadow: 0 16rpx 38rpx rgba(15, 23, 42, 0.065);
	border: 1rpx solid rgba(226, 232, 240, 0.72);
}

.point-list {
	overflow: hidden;
}

.point-row {
	min-height: 106rpx;
	padding: 0 26rpx;
	display: flex;
	align-items: center;
	gap: 18rpx;
	border-bottom: 1rpx solid #EEF2F7;
}

.point-row:last-child {
	border-bottom: 0;
}

.point-row.active {
	background: #EFF6FF;
}

.point-icon {
	width: 58rpx;
	height: 58rpx;
}

.point-main {
	flex: 1;
	min-width: 0;
}

.point-name {
	display: block;
	font-size: 28rpx;
	font-weight: 900;
	color: #0F172A;
}

.point-desc {
	display: block;
	margin-top: 5rpx;
	font-size: 22rpx;
	color: #94A3B8;
}

.point-arrow {
	font-size: 40rpx;
	color: #94A3B8;
}

.history-card {
	padding: 28rpx;
}

.card-head {
	display: flex;
	align-items: flex-start;
	justify-content: space-between;
	gap: 18rpx;
	margin-bottom: 22rpx;
}

.card-title,
.card-subtitle {
	display: block;
}

.card-title {
	font-size: 30rpx;
	font-weight: 900;
	color: #0F172A;
}

.card-subtitle {
	margin-top: 6rpx;
	font-size: 22rpx;
	color: #94A3B8;
}

.period-tabs {
	padding: 6rpx;
	border-radius: 999rpx;
	background: #EEF4FF;
	display: flex;
	gap: 4rpx;
}

.period-tab {
	min-width: 54rpx;
	padding: 10rpx 14rpx;
	border-radius: 999rpx;
	text-align: center;
	font-size: 22rpx;
	font-weight: 850;
	color: #64748B;
}

.period-tab.active {
	color: #FFFFFF;
	background: #2563EB;
}

.chart-card {
	height: 306rpx;
	padding: 28rpx 18rpx 18rpx;
	border-radius: 24rpx;
	background: #F8FAFC;
	border: 1rpx solid #E2E8F0;
	display: flex;
	align-items: flex-end;
	gap: 14rpx;
	box-sizing: border-box;
}

.bar-item {
	flex: 1;
	height: 100%;
	display: flex;
	flex-direction: column;
	align-items: center;
	justify-content: flex-end;
	gap: 10rpx;
	min-width: 0;
}

.bar-track {
	position: relative;
	width: 100%;
	max-width: 32rpx;
	flex: 1;
	border-radius: 999rpx;
	background: #E2E8F0;
	overflow: hidden;
}

.bar-fill {
	position: absolute;
	left: 0;
	right: 0;
	bottom: 0;
	border-radius: 999rpx;
	background: #2563EB;
}

.bar-time {
	font-size: 18rpx;
	color: #94A3B8;
}

.record-table {
	margin-top: 22rpx;
	display: flex;
	flex-direction: column;
}

.record-row {
	min-height: 76rpx;
	display: flex;
	align-items: center;
	border-bottom: 1rpx solid #EEF2F7;
	font-size: 24rpx;
	color: #334155;
}

.record-row text {
	flex: 1;
}

.record-row.head {
	font-weight: 900;
	color: #64748B;
}

.success-text {
	color: #16A34A;
	font-weight: 850;
}

.empty-box {
	min-height: 260rpx;
	display: flex;
	align-items: center;
	justify-content: center;
	text-align: center;
	color: #64748B;
	font-size: 26rpx;
}

.empty-box.warning {
	color: #B45309;
}

@media screen and (min-width: 900px) {
	.page {
		max-width: 1180px;
		margin: 0 auto;
	}
}
</style>
