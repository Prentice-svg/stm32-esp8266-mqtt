<template>
	<view class="page">
		<AppHeader title="历史数据" subtitle="查看监测点变化趋势" />

		<SectionCard>
			<view class="point-list">
				<view
					v-for="point in points"
					:key="point.key"
					class="point-row"
					:class="{ active: activeKey === point.key }"
					@tap="selectPoint(point.key)"
				>
					<view class="point-icon">
						<image v-if="point.iconPath" class="point-icon-img" :src="point.iconPath" mode="aspectFit" />
						<text v-else>{{ point.icon }}</text>
					</view>
					<view class="point-main">
						<text class="point-name">{{ point.label }}</text>
						<text class="point-desc">{{ point.unit || '开关状态' }}</text>
					</view>
					<text class="point-arrow">›</text>
				</view>
			</view>
		</SectionCard>

		<SectionCard :title="activePoint.label + '趋势'" :subtitle="historySubtitle">
			<template #action>
				<view class="period-tabs">
					<text
						v-for="item in periods"
						:key="item"
						class="period-tab"
						:class="{ active: period === item }"
						@tap="selectPeriod(item)"
					>
						{{ item }}
					</text>
				</view>
			</template>
			<view class="chart-card">
				<view class="chart-grid">
					<text class="grid-line"></text>
					<text class="grid-line"></text>
					<text class="grid-line"></text>
				</view>
				<view class="bar-chart">
					<view v-for="item in activeRows" :key="item.time" class="bar-item">
						<view class="bar-track">
							<view class="bar-fill" :style="{ height: `${item.level}%` }"></view>
						</view>
						<text class="bar-time">{{ item.time }}</text>
					</view>
				</view>
				<view class="chart-summary">
					<text class="summary-label">当前</text>
					<text class="summary-value">{{ latestRow.value }}</text>
				</view>
			</view>
			<view v-if="historyError" class="history-error">
				<text>{{ historyError }}</text>
			</view>
		</SectionCard>

		<SectionCard title="历史记录">
			<view v-if="activeRows.length" class="record-table">
				<view class="record-row head">
					<text>时间</text>
					<text>数值</text>
					<text>状态</text>
				</view>
				<view v-for="row in activeRows" :key="row.time" class="record-row">
					<text>{{ row.time }}</text>
					<text>{{ row.value }}</text>
					<text class="success-text">{{ row.status }}</text>
				</view>
			</view>
			<EmptyState v-else title="暂无历史记录" text="当前监测点还没有记录" />
		</SectionCard>

		<AppTabBar active="history" />
	</view>
</template>

<script>
import AppHeader from '../../components/AppHeader.vue'
import AppTabBar from '../../components/AppTabBar.vue'
import SectionCard from '../../components/SectionCard.vue'
import EmptyState from '../../components/EmptyState.vue'
import { monitorPoints, historyMap } from '../../utils/mockData.js'
import { loadHistoryMap } from '../../utils/history.js'
import { loadSettings } from '../../utils/storage.js'
import { buildHistoryRows, getPropertyIdentifier, queryPropertyHistory } from '../../utils/onenet.js'

export default {
	components: {
		AppHeader,
		AppTabBar,
		SectionCard,
		EmptyState
	},
	data() {
		return {
			settings: loadSettings(),
			points: monitorPoints,
			localHistory: {},
			remoteRows: [],
			historyError: '',
			loadingHistory: false,
			activeKey: 'temperature',
			period: '日',
			periods: ['日', '周', '月']
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
			if (this.remoteRows.length) {
				return this.remoteRows
			}
			const rows = this.localHistory[this.activeKey] || []
			return rows.length ? rows : (historyMap[this.activeKey] || [])
		},
		hasRemoteRows() {
			return this.remoteRows.length > 0
		},
		hasLocalRows() {
			return Boolean(this.localHistory[this.activeKey] && this.localHistory[this.activeKey].length)
		},
		historySubtitle() {
			if (this.loadingHistory) return '正在读取 OneNet 历史'
			if (this.hasRemoteRows) return 'OneNet 历史记录'
			return this.hasLocalRows ? '本地真实记录' : '暂无真实记录，显示示例数据'
		},
		latestRow() {
			return this.activeRows[this.activeRows.length - 1] || {
				value: '--'
			}
		}
	},
	methods: {
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
				日: 24 * 60 * 60 * 1000,
				周: 7 * 24 * 60 * 60 * 1000,
				月: 30 * 24 * 60 * 60 * 1000
			}
			return {
				startTime: endTime - (durationMap[this.period] || durationMap.日),
				endTime
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
					this.remoteRows = buildHistoryRows(this.activeKey, rows).slice(-24)
					this.loadingHistory = false
				})
				.catch((error) => {
					this.historyError = error && error.message
						? `OneNet 历史读取失败，已显示本地记录：${error.message}`
						: 'OneNet 历史读取失败，已显示本地记录。'
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

.point-list {
	overflow: hidden;
	border-radius: 24rpx;
}

.point-row {
	min-height: 104rpx;
	padding: 0 8rpx;
	display: flex;
	align-items: center;
	gap: 18rpx;
	border-bottom: 1rpx solid #EEF2F7;
}

.point-row:last-child {
	border-bottom: 0;
}

.point-row.active {
	margin: 0 -8rpx;
	padding: 0 16rpx;
	border-radius: 24rpx;
	background: linear-gradient(90deg, rgba(37, 99, 235, 0.12), rgba(20, 184, 166, 0.08));
}

.point-icon {
	width: 68rpx;
	height: 68rpx;
	border-radius: 50%;
	display: flex;
	align-items: center;
	justify-content: center;
	font-size: 30rpx;
	background: #F8FAFC;
}

.point-icon-img {
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
	font-weight: 850;
	color: #0F172A;
}

.point-desc {
	display: block;
	margin-top: 5rpx;
	font-size: 22rpx;
	color: #94A3B8;
}

.point-arrow {
	font-size: 38rpx;
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
	background: linear-gradient(135deg, #2563EB 0%, #14B8A6 100%);
	box-shadow: 0 8rpx 18rpx rgba(37, 99, 235, 0.22);
}

.chart-card {
	position: relative;
	height: 306rpx;
	padding: 28rpx 28rpx 22rpx;
	border-radius: 28rpx;
	background: linear-gradient(180deg, #FFFFFF 0%, #F8FBFF 100%);
	overflow: hidden;
	border: 1rpx solid #E2E8F0;
	box-sizing: border-box;
}

.chart-grid {
	position: absolute;
	left: 28rpx;
	right: 28rpx;
	top: 40rpx;
	bottom: 62rpx;
	display: flex;
	flex-direction: column;
	justify-content: space-between;
}

.grid-line {
	height: 1rpx;
	background: #E2E8F0;
}

.bar-chart {
	position: absolute;
	left: 40rpx;
	right: 40rpx;
	bottom: 22rpx;
	top: 48rpx;
	display: flex;
	align-items: center;
	justify-content: space-between;
	gap: 20rpx;
}

.bar-item {
	flex: 1;
	height: 100%;
	display: flex;
	flex-direction: column;
	align-items: center;
	justify-content: flex-end;
	gap: 12rpx;
}

.bar-track {
	position: relative;
	width: 34rpx;
	flex: 1;
	border-radius: 999rpx;
	background: #EEF4FF;
	overflow: hidden;
}

.bar-fill {
	position: absolute;
	left: 0;
	right: 0;
	bottom: 0;
	border-radius: 999rpx;
	background: linear-gradient(180deg, #2563EB 0%, #14B8A6 100%);
	box-shadow: 0 8rpx 16rpx rgba(37, 99, 235, 0.18);
}

.bar-time {
	font-size: 20rpx;
	color: #94A3B8;
}

.chart-summary {
	position: absolute;
	right: 24rpx;
	top: 22rpx;
	padding: 10rpx 16rpx;
	border-radius: 999rpx;
	background: rgba(20, 184, 166, 0.1);
	display: flex;
	align-items: baseline;
	gap: 8rpx;
}

.history-error {
	margin-top: 16rpx;
	padding: 16rpx 18rpx;
	border-radius: 20rpx;
	background: rgba(245, 158, 11, 0.1);
	color: #B45309;
	font-size: 22rpx;
	line-height: 1.45;
}

.summary-label {
	font-size: 20rpx;
	color: #64748B;
}

.summary-value {
	font-size: 24rpx;
	font-weight: 900;
	color: #14B8A6;
}

.record-table {
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
	color: #22C55E;
	font-weight: 850;
}
</style>
